#!/bin/bash
set -e
SCRIPT_PATH=$(cd "$(dirname "$0")"; pwd)
PROJECT_ROOT=$(dirname "$(dirname "${SCRIPT_PATH}")")
DEPLOY_DIR="${PROJECT_ROOT}/deploy"
cd "${PROJECT_ROOT}"

TIME=$(date +%Y%m%d_%H%M%S)
PACKAGE_NAME="rag_platform_offline_${TIME}.tar.gz"
IMAGE_TAR="${DEPLOY_DIR}/all_base_images.tar"

echo "1. 拉取并导出全部基础镜像"
docker pull mysql:8.0 redis:7-alpine nginx:alpine milvusdb/milvus:v2.4.0 gcc:13 python:3.10-slim
docker save -o "${IMAGE_TAR}" mysql:8.0 redis:7-alpine nginx:alpine milvusdb/milvus:v2.4.0 gcc:13 python:3.10-slim

echo "2. 打包源码 + 镜像离线包"
tar -zcvf "${DEPLOY_DIR}/${PACKAGE_NAME}" \
  --exclude='.git' \
  --exclude='*.tar' \
  --exclude='*.tar.gz' \
  --exclude='deploy/.env' \
  --exclude='__pycache__' \
  --exclude='.venv' \
  .
echo "打包完成：${DEPLOY_DIR}/${PACKAGE_NAME}"
