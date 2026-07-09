#include "epoll_server.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace gateway {

namespace {

constexpr int kRecvChunkSize = 8192;

}  // namespace

ConnectionPool::ConnectionPool(int max_conn_num) : max_conn_num_(max_conn_num) {}

bool ConnectionPool::addConnection(int fd, int64_t now_ts, std::string& err_msg) {
    if (static_cast<int>(conn_map_.size()) >= max_conn_num_) {
        err_msg = "连接池已满，拒绝新连接";
        return false;
    }
    Connection conn;
    conn.fd = fd;
    conn.last_active_ts = now_ts;
    conn.closed = false;
    conn_map_[fd] = std::move(conn);
    return true;
}

void ConnectionPool::removeConnection(int fd) {
    conn_map_.erase(fd);
}

Connection* ConnectionPool::getConnection(int fd) {
    const auto it = conn_map_.find(fd);
    if (it == conn_map_.end()) {
        return nullptr;
    }
    return &it->second;
}

size_t ConnectionPool::size() const {
    return conn_map_.size();
}

std::vector<int> ConnectionPool::collectIdleConnections(int64_t now_ts) {
    std::vector<int> idle_fds;
    idle_fds.reserve(conn_map_.size());

    for (const auto& item : conn_map_) {
        const int64_t idle_sec = now_ts - item.second.last_active_ts;
        if (idle_sec >= kIdleConnTimeoutSec) {
            idle_fds.push_back(item.first);
        }
    }
    return idle_fds;
}

EpollServer::~EpollServer() {
    stop();
    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

bool EpollServer::init(const GlobalConfig& cfg, std::string& err_msg) {
    cfg_ = cfg;
    conn_pool_ = std::make_unique<ConnectionPool>(cfg_.max_conn_num);
    if (!createListenSocket(err_msg)) {
        return false;
    }
    if (!setupEpoll(err_msg)) {
        return false;
    }
    return true;
}

bool EpollServer::createListenSocket(std::string& err_msg) {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listen_fd_ < 0) {
        err_msg = std::string("创建监听 socket 失败: ") + std::strerror(errno);
        return false;
    }

    int reuse = 1;
    if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        err_msg = std::string("setsockopt SO_REUSEADDR 失败: ") + std::strerror(errno);
        return false;
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(cfg_.listen_port));

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        err_msg = std::string("bind 失败: ") + std::strerror(errno);
        return false;
    }
    if (::listen(listen_fd_, SOMAXCONN) < 0) {
        err_msg = std::string("listen 失败: ") + std::strerror(errno);
        return false;
    }
    return true;
}

bool EpollServer::setupEpoll(std::string& err_msg) {
    epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        err_msg = std::string("epoll_create1 失败: ") + std::strerror(errno);
        return false;
    }

    // 监听 fd 同样使用 ET：accept 必须循环直到 EAGAIN
    if (!addEpollEvent(listen_fd_, EPOLLIN | EPOLLET, err_msg)) {
        return false;
    }
    return true;
}

bool EpollServer::setNonBlocking(int fd, std::string& err_msg) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        err_msg = std::string("fcntl F_GETFL 失败: ") + std::strerror(errno);
        return false;
    }
    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        err_msg = std::string("fcntl 设非阻塞失败: ") + std::strerror(errno);
        return false;
    }
    return true;
}

bool EpollServer::addEpollEvent(int fd, uint32_t events, std::string& err_msg) {
    epoll_event ev {};
    ev.events = events;
    ev.data.fd = fd;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        err_msg = std::string("epoll_ctl ADD 失败: ") + std::strerror(errno);
        return false;
    }
    return true;
}

void EpollServer::removeEpollEvent(int fd) {
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void EpollServer::run() {
    running_ = true;
    epoll_event events[kEpollMaxEvents];

    std::cout << "[EpollServer] 启动成功，监听端口=" << cfg_.listen_port
              << " max_conn=" << cfg_.max_conn_num << " ET模式" << std::endl;

    while (running_) {
        const int event_count =
            ::epoll_wait(epoll_fd_, events, kEpollMaxEvents, kEpollWaitTimeoutMs);
        const int64_t now_ts = nowSeconds();

        if (event_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "[EpollServer] epoll_wait 失败: " << std::strerror(errno) << std::endl;
            break;
        }

        for (int i = 0; i < event_count; ++i) {
            const int fd = events[i].data.fd;
            const uint32_t ev = events[i].events;

            if (fd == listen_fd_) {
                if (ev & EPOLLIN) {
                    handleAccept(now_ts);
                }
                continue;
            }

            if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                closeConnection(fd);
                continue;
            }

            if (ev & EPOLLIN) {
                handleClientRead(fd, now_ts);
            }
        }

        onTick(now_ts);
    }
}

void EpollServer::stop() {
    running_ = false;
}

void EpollServer::handleAccept(int64_t now_ts) {
    /*
     * ET 边沿触发 accept 规范：
     * - 可读事件只沿上升沿通知一次；
     * - 必须 while(true) accept 直到返回 EAGAIN/EWOULDBLOCK；
     * - 否则 backlog 中剩余连接不会被再次唤醒。
     */
    while (running_) {
        sockaddr_in client_addr {};
        socklen_t client_len = sizeof(client_addr);
        const int client_fd =
            ::accept4(listen_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len,
                      SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "[EpollServer] accept 失败: " << std::strerror(errno) << std::endl;
            break;
        }

        std::string err_msg;
        if (!conn_pool_->addConnection(client_fd, now_ts, err_msg)) {
            std::cerr << "[EpollServer] " << err_msg << std::endl;
            ::close(client_fd);
            continue;
        }

        if (!addEpollEvent(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP, err_msg)) {
            std::cerr << "[EpollServer] " << err_msg << std::endl;
            closeConnection(client_fd);
            continue;
        }

        char ip_buf[INET_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &client_addr.sin_addr, ip_buf, sizeof(ip_buf));
        std::cout << "[EpollServer] 新连接 fd=" << client_fd << " ip=" << ip_buf
                  << " pool_size=" << conn_pool_->size() << std::endl;
    }
}

void EpollServer::handleClientRead(int fd, int64_t now_ts) {
    Connection* conn = conn_pool_->getConnection(fd);
    if (conn == nullptr) {
        return;
    }

    /*
     * ET 边沿触发 read 规范：
     * - 收到 EPOLLIN 后必须循环 recv 直到 EAGAIN；
     * - 若只读一次就返回，缓冲区剩余字节不会触发新事件，导致半包长期滞留。
     */
    while (true) {
        char chunk[kRecvChunkSize];
        const ssize_t n = ::recv(fd, chunk, sizeof(chunk), 0);
        if (n > 0) {
            conn->read_buffer.append(chunk, static_cast<size_t>(n));
            conn->last_active_ts = now_ts;
            continue;
        }
        if (n == 0) {
            closeConnection(fd);
            return;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        closeConnection(fd);
        return;
    }
}

void EpollServer::closeConnection(int fd) {
    removeEpollEvent(fd);
    ::close(fd);
    conn_pool_->removeConnection(fd);
}

void EpollServer::onTick(int64_t now_ts) {
    const std::vector<int> idle_fds = conn_pool_->collectIdleConnections(now_ts);
    for (const int fd : idle_fds) {
        closeConnection(fd);
    }
    if (!idle_fds.empty()) {
        std::cout << "[EpollServer] 心跳回收空闲连接 count=" << idle_fds.size()
                  << " idle_timeout_sec=" << kIdleConnTimeoutSec << std::endl;
    }
}

int64_t EpollServer::nowSeconds() {
    using clock = std::chrono::system_clock;
    return std::chrono::duration_cast<std::chrono::seconds>(clock::now().time_since_epoch()).count();
}

}  // namespace gateway
