#!/bin/bash
set -e
SCRIPT_PATH=$(cd "$(dirname "$0")";pwd)
DEPLOY_DIR=$(dirname ${SCRIPT_PATH})
cd ${DEPLOY_DIR}

echo "===== 清理7天前全平台日志 ====="
for svc in nginx django_backend cpp_gateway agent_engine; do
  docker compose exec -T "${svc}" find /var/log/nginx /app/logs -type f -mtime +7 -delete 2>/dev/null || true
done
echo "日志清理完成"
