FROM gcc:13

# 预装编译与调试依赖（gcc:13 基于 Debian，源文件可能在 sources.list 或 debian.sources）
RUN set -eux; \
    for f in /etc/apt/sources.list /etc/apt/sources.list.d/debian.sources; do \
        if [ -f "$f" ]; then \
            sed -i 's|deb.debian.org|mirrors.tuna.tsinghua.edu.cn|g; s|security.debian.org|mirrors.tuna.tsinghua.edu.cn|g' "$f"; \
        fi; \
    done; \
    apt-get update && apt-get install -y --no-install-recommends \
        cmake \
        make \
        curl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# 镜像构建阶段编译 gateway_server，安装到 /usr/local/bin（避免 /app 卷挂载覆盖二进制）
RUN mkdir -p build && cd build && cmake .. && make -j"$(nproc)" && \
    cp gateway_server /usr/local/bin/gateway_server

EXPOSE 8080

CMD ["/usr/local/bin/gateway_server"]
