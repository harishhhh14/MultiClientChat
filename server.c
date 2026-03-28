#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10

int clients[MAX_CLIENTS];
int client_count = 0;

void broadcast(char *message, int sender_sock) {
    for(int i = 0; i < client_count; i++) {
        if(clients[i] != sender_sock) {
            send(clients[i], message, strlen(message), 0);
        }
    }
}

void* handle_client(void* socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int read_size = recv(sock, buffer, 1024, 0);

        if(read_size <= 0) {
            printf("Client disconnected\n");
            close(sock);
            break;
        }

        printf("Client: %s\n", buffer);

        broadcast(buffer, sock);
    }

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server started...\n");

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        printf("New client connected\n");

        clients[client_count++] = new_socket;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void*)&new_socket);
    }

    return 0;
}