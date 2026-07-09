#!/bin/bash
set -e
SCRIPT_PATH=$(cd "$(dirname "$0")"; pwd)
DEPLOY_DIR=$(dirname "${SCRIPT_PATH}")
cd "${DEPLOY_DIR}"

echo "===== 批量文档导入 Milvus 向量库 ====="
docker compose exec -T django_backend python /app/scripts/batch_import_kb.py
echo "===== 批量导入完成 ====="
