#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // PF_INET
#include <arpa/inet.h>
#include <pthread.h>
#include "proto.h"
#include "server.h"

// Global variables and main sockets
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;


void catch_ctrl_c_and_exit(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\033[0;31m\nClosing socket: %d\n", root->data);
        printf("\033[0m");
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->next;
        free(tmp);
    }
    printf("\033[1;31mServer shutdown.\n");
    printf("\033[0m");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = root->next;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
            printf("[>] \033[0;36mSent to sockfd %d: ", tmp->data);
            printf("\033[0m");
            printf("\"%s\" \n", tmp_buffer);
            
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->next;
    }
}

void client_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientList *np = (ClientList *)p_client;

    // Setting up name
    if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("%s didn't input name.\n", np->ip);
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, LENGTH_NAME);
        printf("[!] \033[0;33m%s [%s:%d] joined the chatroom.\n", np->name, np->ip, np->data);
        printf("\033[0m");
        sprintf(send_buffer, "\033[0;33m%s [%s] joined the chatroom.\033[0m", np->name, np->ip);
        send_to_all_clients(np, send_buffer);
    }

    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            // decrypt
            char key[] = "OPERATINGSYSTEMS";
            int i;
            for(i = 0; i < strlen(recv_buffer); i++) {
                recv_buffer[i] = recv_buffer[i] ^ key[i % (sizeof(key)/sizeof(char))];
            }

            sprintf(send_buffer, "%s[%s]: %s ", np->name, np->ip, recv_buffer);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("[!] \033[0;33m%s [%s:%d] left the chatroom.\n", np->name, np->ip, np->data);
            printf("\033[0m");
            sprintf(send_buffer, "\033[0;33m%s [%s] left the chatroom.\033[0m", np->name, np->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(np, send_buffer);
    }

    // Remove Node
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->previous;
        now->next = NULL;
    } else { // remove a middle node
        np->previous->next = np->next;
        np->next->previous = np->previous;
    }
    free(np);
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    // Create socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockfd == -1) {
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
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(8080);

    // Binding and Listening
    bind(server_sockfd, (struct sockaddr *)&server_info, server_address_len);
    listen(server_sockfd, 5);

    // Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &server_address_len);
    printf("\033[0;32mServer started on ");
    printf("\033[0;35m[");
    printf("\033[0m");
    printf("%s:%d", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("\033[0;35m]");
    printf("\033[0m\n\n");

    // Creating the first node, which is the server
    root = new_node(server_sockfd, inet_ntoa(server_info.sin_addr));
    now = root;

    // Server running
    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &client_address_len);

        // Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &client_address_len);
        printf("[=] ");
        printf("\033[0;32mEstablished connection with ");
        printf("\033[0m%s:%d.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
        

        // Append linked list for clients
        ClientList *c = new_node(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->previous = now;
        now->next = c;
        now = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
            // Pthread error
        printf("\033[0;31m[!] An error has ocurred.\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}