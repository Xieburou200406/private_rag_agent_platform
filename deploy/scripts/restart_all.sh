#!/bin/bash
set -e
SCRIPT_PATH=$(cd "$(dirname "$0")";pwd)
DEPLOY_DIR=$(dirname ${SCRIPT_PATH})
cd ${DEPLOY_DIR}

echo "===== 停止全部容器 ====="
docker compose down
echo "===== 重建镜像并后台启动 ====="
docker compose build --no-cache
docker compose up -d
echo "===== 平台重启完成，查看实时日志：docker compose logs -f ====="