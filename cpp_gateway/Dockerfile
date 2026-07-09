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

# 开发占位：编译 health stub，正式网关完成后改为 cmake 编译 gateway_server
RUN gcc -O2 -o /usr/local/bin/dev_health_stub dev_health_stub.c

# 生产阶段取消注释以下两行，并将 CMD 改为 ./build/gateway_server
# RUN mkdir -p build && cd build && cmake .. && make -j"$(nproc)"

EXPOSE 8080

CMD ["/usr/local/bin/dev_health_stub"]
