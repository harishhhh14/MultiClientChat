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
pthread_mutex_t lock;

void broadcast(char *message, int sender_sock) {
    pthread_mutex_lock(&lock);

    for(int i = 0; i < client_count; i++) {
        if(clients[i] != sender_sock) {
            send(clients[i], message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

void* server_send(void* arg) {
    char message[1024];

    while(1) {
        fgets(message, sizeof(message), stdin);

        pthread_mutex_lock(&lock);

        for(int i = 0; i < client_count; i++) {
            send(clients[i], message, strlen(message), 0);
        }

        pthread_mutex_unlock(&lock);
    }

    return NULL;
}


void* handle_client(void* arg) {
    int sock = *(int*)arg;
    free(arg);

    char buffer[1024];

    while(1) {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(sock, buffer, sizeof(buffer), 0);

        if(bytes <= 0) {
            printf("Client disconnected\n");
            close(sock);
            break;
        }

        printf("Client: %s", buffer);

        broadcast(buffer, sock);
    }

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    pthread_mutex_init(&lock, NULL);


    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket created\n");


    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    printf("Bind successful\n");

   
    listen(server_fd, 5);
    printf("Server listening...\n");

    pthread_t send_thread;
    pthread_create(&send_thread, NULL, server_send, NULL);

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        printf("New client connected\n");

        pthread_mutex_lock(&lock);
        clients[client_count++] = new_socket;
        pthread_mutex_unlock(&lock);

        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, (void*)new_sock);
    }

    return 0;
}
