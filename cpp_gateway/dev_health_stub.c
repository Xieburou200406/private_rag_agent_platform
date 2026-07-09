/* 开发占位：网关 C++ 源码完成前，提供 /gateway/v1/health 探测 */
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *RESPONSE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Connection: close\r\n"
    "\r\n"
    "{\"code\":200,\"msg\":\"gateway dev stub\",\"data\":{\"service\":\"cpp_gateway\"},\"request_id\":null}";

int main(void) {
    int port = 8080;
    const char *env = getenv("GATEWAY_LISTEN_PORT");
    if (env && *env) {
        port = atoi(env);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(server_fd, 32) < 0) {
        perror("listen");
        return 1;
    }

    printf("cpp_gateway dev stub listening on %d\n", port);
    fflush(stdout);

    for (;;) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) {
            continue;
        }
        char buf[512];
        recv(client, buf, sizeof(buf) - 1, 0);
        send(client, RESPONSE, strlen(RESPONSE), 0);
        close(client);
    }
}
