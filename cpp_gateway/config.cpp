#include "config.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace gateway {

namespace {

std::string trim(const std::string& text) {
    const auto begin = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto end = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    if (begin >= end) {
        return "";
    }
    return std::string(begin, end);
}

std::string stripInlineComment(const std::string& line) {
    bool in_single_quote = false;
    bool in_double_quote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (ch == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
        } else if (ch == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
        } else if (ch == '#' && !in_single_quote && !in_double_quote) {
            return line.substr(0, i);
        }
    }
    return line;
}

bool parseEnvFile(const std::string& env_path, std::unordered_map<std::string, std::string>& kv_map,
                  std::string& err_msg) {
    std::ifstream input(env_path);
    if (!input.is_open()) {
        err_msg = "无法打开 .env 文件: " + env_path;
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        line = trim(stripInlineComment(line));
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        const std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        if (!value.empty() && ((value.front() == '"' && value.back() == '"') ||
                               (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }
        if (!key.empty()) {
            kv_map[key] = value;
        }
    }
    return true;
}

int readInt(const std::unordered_map<std::string, std::string>& kv_map, const std::string& key,
            int default_value) {
    const auto it = kv_map.find(key);
    if (it == kv_map.end() || it->second.empty()) {
        return default_value;
    }
    return std::atoi(it->second.c_str());
}

std::string readString(const std::unordered_map<std::string, std::string>& kv_map,
                       const std::string& key, const std::string& default_value) {
    const auto it = kv_map.find(key);
    if (it == kv_map.end() || it->second.empty()) {
        return default_value;
    }
    return it->second;
}

void fillConfigFromMap(const std::unordered_map<std::string, std::string>& kv_map,
                       GlobalConfig& cfg) {
    cfg.listen_port = readInt(kv_map, "GATEWAY_LISTEN_PORT", cfg.listen_port);
    cfg.max_qps = readInt(kv_map, "GATEWAY_MAX_QPS", cfg.max_qps);
    cfg.max_conn_num = readInt(kv_map, "MAX_CONN_NUM", cfg.max_conn_num);
    cfg.timeout_ms = readInt(kv_map, "GATEWAY_TIMEOUT_MS", cfg.timeout_ms);

    cfg.milvus_host = readString(kv_map, "MILVUS_HOST", cfg.milvus_host);
    cfg.milvus_port = readInt(kv_map, "MILVUS_PORT", cfg.milvus_port);

    cfg.django_host = readString(kv_map, "DJANGO_HOST", cfg.django_host);
    cfg.django_port = readInt(kv_map, "DJANGO_LISTEN_PORT", cfg.django_port);

    cfg.perf_log_path = readString(kv_map, "GATEWAY_PERF_LOG", cfg.perf_log_path);
    if (cfg.perf_log_path.empty()) {
        cfg.perf_log_path = "logs/gateway/perf.log";
    }
}

std::vector<std::string> buildCandidatePaths() {
    std::vector<std::string> paths;
    const char* override_path = std::getenv("GATEWAY_ENV_FILE");
    if (override_path != nullptr && override_path[0] != '\0') {
        paths.emplace_back(override_path);
    }
    paths.emplace_back("../deploy/.env");
    paths.emplace_back("deploy/.env");
    paths.emplace_back(".env");
    return paths;
}

}  // namespace

bool GlobalConfigLoader::loadFromFile(const std::string& env_path, GlobalConfig& out_cfg,
                                      std::string& err_msg) {
    std::unordered_map<std::string, std::string> kv_map;
    if (!parseEnvFile(env_path, kv_map, err_msg)) {
        return false;
    }
    fillConfigFromMap(kv_map, out_cfg);
    return true;
}

bool GlobalConfigLoader::load(GlobalConfig& out_cfg, std::string& err_msg) {
    const std::vector<std::string> candidates = buildCandidatePaths();
    for (const std::string& path : candidates) {
        std::ifstream probe(path);
        if (!probe.is_open()) {
            continue;
        }
        probe.close();
        if (loadFromFile(path, out_cfg, err_msg)) {
            return true;
        }
        return false;
    }
    err_msg = "未找到可用 .env 文件，请配置 deploy/.env 或设置 GATEWAY_ENV_FILE";
    return false;
}

bool GlobalConfigLoader::validate(const GlobalConfig& cfg, std::string& err_msg) {
    if (cfg.listen_port <= 0 || cfg.listen_port > 65535) {
        err_msg = "GATEWAY_LISTEN_PORT 无效";
        return false;
    }
    if (cfg.max_qps <= 0) {
        err_msg = "GATEWAY_MAX_QPS 无效";
        return false;
    }
    if (cfg.max_conn_num <= 0) {
        err_msg = "MAX_CONN_NUM 无效";
        return false;
    }
    if (cfg.timeout_ms <= 0) {
        err_msg = "GATEWAY_TIMEOUT_MS 无效";
        return false;
    }
    if (cfg.milvus_host.empty()) {
        err_msg = "MILVUS_HOST 未配置";
        return false;
    }
    if (cfg.milvus_port <= 0) {
        err_msg = "MILVUS_PORT 无效";
        return false;
    }
    if (cfg.django_port <= 0) {
        err_msg = "DJANGO_LISTEN_PORT 无效";
        return false;
    }
    return true;
}

}  // namespace gateway

gateway::GlobalConfig g_global_config;
