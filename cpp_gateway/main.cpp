#include "config.hpp"
#include "epoll_server.hpp"

#include <csignal>
#include <iostream>
#include <string>

namespace {

gateway::EpollServer* g_epoll_server = nullptr;

void handleSignal(int) {
    if (g_epoll_server != nullptr) {
        g_epoll_server->stop();
    }
}

}  // namespace

int main() {
    std::string err_msg;
    if (!gateway::GlobalConfigLoader::load(g_global_config, err_msg)) {
        std::cerr << "[gateway] 加载配置失败: " << err_msg << std::endl;
        return 1;
    }
    if (!gateway::GlobalConfigLoader::validate(g_global_config, err_msg)) {
        std::cerr << "[gateway] 配置校验失败: " << err_msg << std::endl;
        return 1;
    }

    gateway::EpollServer server;
    g_epoll_server = &server;
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    if (!server.init(g_global_config, err_msg)) {
        std::cerr << "[gateway] 初始化 epoll 服务失败: " << err_msg << std::endl;
        return 1;
    }

    server.run();
    std::cout << "[gateway] 服务已退出" << std::endl;
    return 0;
}
