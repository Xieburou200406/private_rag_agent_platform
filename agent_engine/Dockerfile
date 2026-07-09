FROM python:3.10-slim

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    AGENT_LISTEN_PORT=8001

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt \
    -i https://pypi.tuna.tsinghua.edu.cn/simple

COPY . .

EXPOSE 8001

# 内网服务，仅 Django 容器调用 /agent/internal/v1/*
CMD ["sh", "-c", "python main.py --host 0.0.0.0 --port ${AGENT_LISTEN_PORT:-8001}"]
