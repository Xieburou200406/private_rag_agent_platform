#pragma once

#include <cstdint>
#include <string>

/**
 * @file config.hpp
 * @brief 网关全局配置结构体 GlobalConfig 与 .env 加载器
 *
 * 所有运行参数从 deploy/.env 读取，禁止在业务代码中硬编码 IP、端口、并发阈值。
 */

namespace gateway {

/** 网关运行时配置，字段与 deploy/.env 键名一一对应 */
struct GlobalConfig {
    int listen_port = 0;
    int max_qps = 0;
    int max_conn_num = 0;
    int timeout_ms = 0;

    std::string milvus_host;
    int milvus_port = 0;

    std::string django_host;
    int django_port = 0;

    /** 性能日志输出路径，相对网关工作目录 */
    std::string perf_log_path;
};

/**
 * @brief .env 配置加载器
 */
class GlobalConfigLoader {
public:
    /**
     * @brief 按优先级自动查找 .env 并加载到 GlobalConfig
     *
     * 查找顺序：
     *   1. 环境变量 GATEWAY_ENV_FILE
     *   2. ../deploy/.env（容器 /app 挂载场景）
     *   3. deploy/.env（项目根目录启动）
     *   4. .env
     */
    static bool load(GlobalConfig& out_cfg, std::string& err_msg);

    static bool loadFromFile(const std::string& env_path,
                             GlobalConfig& out_cfg,
                             std::string& err_msg);

    static bool validate(const GlobalConfig& cfg, std::string& err_msg);
};

}  // namespace gateway

/** 全局配置，启动时一次性加载，各模块只读 */
extern gateway::GlobalConfig g_global_config;
