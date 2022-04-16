#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <csignal>


void sighandle(int sig) {
    if (SIGINT == sig) {
        printf("receive SIGINT signal\n");
        exit(-1);
    }

}

void bind(int socket) {

}

#define PORT 8999
#define MAX_EVENTS 1024
#define READ_BUFF_SIZE 1024

void print_client_info(sockaddr_in *pIn);

int main() {

    int status;

    if (SIG_ERR == signal(SIGINT, sighandle)) {
        perror("signal SIGINT failed\n");
        exit(-1);
    }

    int epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        perror("create epoll fd failed");
        exit(-1);
    }
    printf("epoll fd: %d\n", epoll_fd);

    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == server_sock) {
        perror("create server socket failed");
        exit(-1);
    }
    printf("server sock fd: %d\n", server_sock);

    int opt = 1;
    if (-1 == setsockopt(server_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("set socket opt SO_REUSEPORT failed");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    status = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (0 != status) {
        perror("bind failed");
        exit(status);
    }

    status = listen(server_sock, 10);
    if (-1 == status) {
        perror("listen failed");
        exit(-1);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&client_addr, 0, client_addr_len);


    struct epoll_event event;
    struct epoll_event ready_socket_event[MAX_EVENTS];

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = server_sock;

    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &event)) {
        perror("epoll ctl failed");
        exit(-1);
    }

    char buff[READ_BUFF_SIZE] = {0};

    for(;;) {
        printf("wait client event\n");
        int ready_socket_num = epoll_wait(epoll_fd, ready_socket_event, MAX_EVENTS, -1);
        for (int i = 0; i < ready_socket_num; ++i) {
            // accept
            if (server_sock == ready_socket_event[i].data.fd) {
                printf("new client connected\n");
                int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
                if (client_sock < 0) {
                    perror("accept failed");
                    exit(-1);
                }
                printf("new client sock fd: %d\n", client_sock);

                int flags = fcntl(client_sock, F_GETFL, 0);
                if (-1 == fcntl(client_sock, F_SETFL, flags | O_NONBLOCK)) {
                    perror("set client socket O_NONBLOCK failed");
                    exit(-1);
                }

                print_client_info(&client_addr);

                event.data.fd = client_sock;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event);
            } else if (ready_socket_event[i].events & EPOLLIN) {
                memset(buff, 0, READ_BUFF_SIZE);

                int read_size = read(ready_socket_event[i].data.fd, buff, sizeof(buff));
                if (-1 == read_size) {
                    perror("read failed");
                    exit(-1);
                }  else if (0 == read_size) {
                    printf("client quit\n");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ready_socket_event[i].data.fd, &event);
                    close(ready_socket_event[i].data.fd);
                } else if ( 0 < read_size) {
                    printf("client say: %s\n", buff);
                }
            } else if (ready_socket_event[i].events & EPOLLOUT) {
                printf("write event\n");
            } else {
                printf("unknown event\n");
            }
        }
    }

    return 0;
}

void print_client_info(sockaddr_in *client_addr) {

    int port = htons(client_addr->sin_port);

    char ip[16];
    memset(ip, 0, sizeof(ip));
    inet_ntop(AF_INET, &(client_addr->sin_addr.s_addr), ip, sizeof(ip));
    printf("client connected, ip: %s, port: %d\n", ip, port);
}
