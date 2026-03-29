#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>

#define PORT 8080

int sock;


void* receive_msg(void* arg) {
    char buffer[1024];

    while(1) {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(sock, buffer, sizeof(buffer), 0);

        if(bytes <= 0) {
            printf("\nDisconnected from server\n");
            break;
        }

        printf("\nMessage: %s\nYou: ", buffer);
        fflush(stdout);
    }

    return NULL;
}

void* send_msg(void* arg) {
    char buffer[1024];

    while(1) {
        printf("You: ");
        fgets(buffer, sizeof(buffer), stdin);

        if(strncmp(buffer, "stop", 4) == 0) {
            close(sock);
            exit(0);
        }

        send(sock, buffer, strlen(buffer), 0);
    }

    return NULL;
}

int main() {
    struct sockaddr_in serv_addr;

    // 🔹 Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        printf("Socket creation failed\n");
        return -1;
    }

    printf("Client socket created\n");

    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);


    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

   
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    printf("Connected to server\n");

    pthread_t send_thread, recv_thread;

   
    pthread_create(&recv_thread, NULL, receive_msg, NULL);
    pthread_create(&send_thread, NULL, send_msg, NULL);

    
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    return 0;
}
