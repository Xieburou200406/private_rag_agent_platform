FROM python:3.10-slim

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    DJANGO_LISTEN_PORT=8000

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt \
    -i https://pypi.tuna.tsinghua.edu.cn/simple

COPY . .

EXPOSE 8000

# 端口从 deploy/.env 的 DJANGO_LISTEN_PORT 读取
CMD ["sh", "-c", "gunicorn --bind 0.0.0.0:${DJANGO_LISTEN_PORT:-8000} --workers 2 --timeout 120 rag_project.wsgi:application"]
