#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void fatal_error()
{
    write(2, "Fatal error\n", 12);
    exit(1);
}

int id_by_sock[65536];
int max_sock = 0;
int next_id = 0;
fd_set active_socks, ready_for_read, ready_for_write;
char buf_for_read[42*4096], buf_str[42*4096], buf_for_write[42*4096+42];

void send_all(int except_sock) {
    int len = strlen(buf_for_write);
    for (int sel_sock = 0; sel_sock <= max_sock; sel_sock++)
        if (FD_ISSET(sel_sock, &ready_for_write) && sel_sock != except_sock) {
            send(sel_sock, buf_for_write, len, 0);
        }
}

int main(int ac, char **av) {
    if (ac != 2) {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }
    int port = atoi(av[1]); (void) port;

    bzero(&id_by_sock, sizeof(id_by_sock));
    FD_ZERO(&active_socks);

    // start server
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) fatal_error();

    max_sock = server_sock;
    FD_SET(server_sock, &active_socks);

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = (1 << 24) | 127;
    addr.sin_port = (port >> 8) | (port << 8);

    if ((bind(server_sock, (const struct sockaddr *)&addr, sizeof(addr))) < 0)
        fatal_error();
    if (listen(server_sock, SOMAXCONN) < 0)
        fatal_error();

    while (1) {
        ready_for_read = ready_for_write = active_socks;
        if (select(max_sock + 1, &ready_for_read, &ready_for_write, NULL, NULL) < 0)
            continue ;

        for (int sel_sock = 0; sel_sock <= max_sock; sel_sock++) {

            if (FD_ISSET(sel_sock, &ready_for_read) && sel_sock == server_sock) {
                int client_sock = accept(server_sock, (struct sockaddr *)&addr, &addr_len);\
                if (client_sock < 0) continue ;

                max_sock = (client_sock > max_sock) ? client_sock : max_sock;
                id_by_sock[client_sock] = next_id++;
                FD_SET(client_sock, &active_socks);

                sprintf(buf_for_write, "server: client %d just arrived\n", id_by_sock[client_sock]);
                send_all(client_sock);
                break ;
            }

            if (FD_ISSET(sel_sock, &ready_for_read) && sel_sock != server_sock) {

                int read_res = recv(sel_sock, buf_for_read, 42*4096, 0);

                if (read_res <= 0) {
                    sprintf(buf_for_write, "server: client %d just left\n", id_by_sock[sel_sock]);
                    send_all(sel_sock);
                    FD_CLR(sel_sock, &active_socks);
                    close(sel_sock);
                    break ;
                }
                else {
                    for (int i = 0, j = 0; i < read_res; i++, j++) {
                        buf_str[j] = buf_for_read[i];
                        if (buf_str[j] == '\n') {
                            buf_str[j] = '\0';
                            sprintf(buf_for_write, "client %d: %s\n", id_by_sock[sel_sock], buf_str);
                            send_all(sel_sock);
                            j = -1;
                        }
                    }
                }
            }
        }
    }
    return (0);
}
