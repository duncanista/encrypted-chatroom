#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "proto.h"
#include "string.h"

// global variables and atomic signal as flag
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[LENGTH_NAME] = {};

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void receive_message() {
    char message[LENGTH_SEND] = {};

    

    while (1) {
        int receive = recv(sockfd, message, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", message);
            printf("\r%s", "> ");
            fflush(stdout);
        } else if (receive == 0) {
            break;
        } else { 
            // -1 
        }
    }
}

void send_message() {
    char message[LENGTH_MSG] = {};
    while (1) {
        printf("\r%s", "> ");
        fflush(stdout);
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            int i;
            for (i = 0; i < LENGTH_MSG; i++) { // trim \n
                if (message[i] == '\n') {
                    message[i] = '\0';
                    break;
                }
            }
            if (strlen(message) == 0) {
                printf("\r%s", "> ");
                fflush(stdout);
            } else {
                break;
            }
        }

        // encrypting
        char key[] = "OPERATINGSYSTEMS";
        int i;
        for(i = 0; i < strlen(message); i++) {
            message[i] = message[i] ^ key[i % (sizeof(key)/sizeof(char))];
        }
	   
        send(sockfd, message, LENGTH_MSG, 0);
        if (strcmp(message, "/exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    // Setting up nickname
    printf("\033[1;36mPlease enter your name: ");
    printf("\033[0m");

    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        for (int i = 0; i < LENGTH_MSG; i++) { // trim \n
            if (nickname[i] == '\n') {
                nickname[i] = '\0';
                break;
            }
        }
    }

    // Check if nickname lenght is between 3 and 16
    if (strlen(nickname) <= 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("\n\033[1;33m[!] Name lenght must be at least 3 characters and less than 16.\n");
        printf("\033[0;31mDisconnecting...\n");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Creating the socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("\033[0;31mFailed to create socket.\n");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;
    int server_address_len = sizeof(server_info);
    int client_address_len = sizeof(client_info);
    memset(&server_info, 0, server_address_len);
    memset(&client_info, 0, client_address_len);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8080);

    // Connecting to server
    int err = connect(sockfd, (struct sockaddr *)&server_info, server_address_len);
    if (err == -1) {
        printf("\033[0;31mConnection unsuccesful\n");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }
    
    // Welcome message
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &client_address_len);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &server_address_len);
    // [!] Connected to server [IP:PORT]
    printf("\n[!] \033[0;32mConnected to server \033[0;35m[\033[0m");
    printf("%s:%d", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("\033[0;35m]\033[0m\n");

    // Assigned IP: [IP:PORT]
    printf("Assigned IP \033[0;35m[\033[0m%s:%d\033[0;35m]\033[0m\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, LENGTH_NAME, 0);

    pthread_t send_message_thread;
    if (pthread_create(&send_message_thread, NULL, (void *) send_message, NULL) != 0) {
        // Pthread error
        printf("\033[0;31m[!] An error has ocurred.\n");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    pthread_t receive_message_thread;
    if (pthread_create(&receive_message_thread, NULL, (void *) receive_message, NULL) != 0) {
        // Pthread error
        printf("\033[0;31m[!] An error has ocurred.\n");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\033[0;31m\nLeaving chatroom...\n");
            printf("\033[0m");
            break;
        }
    }

    close(sockfd);
    return 0;
}