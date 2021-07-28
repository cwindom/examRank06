#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
int id[1024 * 64];
int max = 0, next_id = 0;
fd_set readyWrite, readyRead, active;
char bufRead[42 * 4096], str[42 * 4096], bufWrite[42 * 4097];
void fatal_error(){
    write(2, "Fatal error\n", 12);
    exit(1);
}
void send_all(int es){
    for(int i = 0; i <= max; i++)
        if(FD_ISSET(i, &readyWrite) && i != es)
            send(i, bufWrite, strlen(bufWrite), 0);
}
int main(int ac, char **av){
    if (ac != 2){
        write(2, "Error arg\n", strlen("Error arg\n"));
        exit(1);
    }
    int port = atoi(av[1]);
    (void)port;

    bzero(&id, sizeof(id));
    FD_ZERO(&active);

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSock < 0)
        fatal_error();
    max = serverSock;
    FD_SET(serverSock, &active);

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = (1 << 24) | 127; //127.0.0.1
    addr.sin_port = (port >> 8) | (port << 8);

    if(bind(serverSock, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
        fatal_error();
    if(listen(serverSock, 128) < 0)
        fatal_error();

    while(1){
        readyWrite = readyRead = active;
        if(select(max + 1, &readyRead, &readyWrite, NULL, NULL) < 0)
            continue;
        for(int s = 0; s <= max; s++){
            if(FD_ISSET(s, &readRead) && s == serverSock){

            }
            if(FD_ISSET(s, &readRead) && s != serverSock){
                int res = recv(s, bufRead, 42 * 4096, 0);
                if(res <= 0){

                }else{

                }
            }
        }
    }
}