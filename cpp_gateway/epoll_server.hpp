#pragma once

#include "config.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace gateway {

/** 空闲连接超时秒数，超过后 ConnectionPool 自动回收 */
constexpr int kIdleConnTimeoutSec = 300;

/** 单次 epoll_wait 最大事件数 */
constexpr int kEpollMaxEvents = 1024;

/** epoll_wait 超时毫秒，用于驱动心跳扫描 */
constexpr int kEpollWaitTimeoutMs = 1000;

/**
 * @brief 单条 TCP 连接上下文
 */
struct Connection {
    int fd = -1;
    int64_t last_active_ts = 0;
    std::string read_buffer;
    bool closed = false;
};

/**
 * @brief TCP 连接池
 *
 * 维护 fd -> Connection 映射，提供增删、活跃时间更新、空闲回收。
 */
class ConnectionPool {
public:
    explicit ConnectionPool(int max_conn_num);

    bool addConnection(int fd, int64_t now_ts, std::string& err_msg);
    void removeConnection(int fd);
    Connection* getConnection(int fd);
    size_t size() const;

    /**
     * @brief 找出空闲超过 kIdleConnTimeoutSec 的连接 fd（不直接 close）
     * @return 待回收 fd 列表，由 EpollServer 统一 epoll_del + close
     */
    std::vector<int> collectIdleConnections(int64_t now_ts);

private:
    int max_conn_num_;
    std::unordered_map<int, Connection> conn_map_;
};

/**
 * @brief epoll ET 边沿触发 Reactor 服务器（第一阶段：仅接入与连接管理）
 *
 * ET 模式要点：
 * 1. 所有 socket 必须设为非阻塞；
 * 2. EPOLLIN 触发后必须循环 read/accept 直到 EAGAIN，否则剩余数据不会再次通知；
 * 3. 新连接、读事件、对端关闭(EPOLLRDHUP) 均需显式处理。
 */
class EpollServer {
public:
    EpollServer() = default;
    ~EpollServer();

    EpollServer(const EpollServer&) = delete;
    EpollServer& operator=(const EpollServer&) = delete;

    bool init(const GlobalConfig& cfg, std::string& err_msg);
    void run();
    void stop();

private:
    bool createListenSocket(std::string& err_msg);
    bool setupEpoll(std::string& err_msg);
    bool setNonBlocking(int fd, std::string& err_msg);
    bool addEpollEvent(int fd, uint32_t events, std::string& err_msg);
    void removeEpollEvent(int fd);

    void handleAccept(int64_t now_ts);
    void handleClientRead(int fd, int64_t now_ts);
    void closeConnection(int fd);

    void onTick(int64_t now_ts);

    static int64_t nowSeconds();

    GlobalConfig cfg_;
    int listen_fd_ = -1;
    int epoll_fd_ = -1;
    bool running_ = false;

    std::unique_ptr<ConnectionPool> conn_pool_;
};

}  // namespace gateway
