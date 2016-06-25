#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define LENGTH 512 

int main(int argc, char *argv[]){
    if(argc<5){
        puts("Usage: ./client <IP> <PORT> <FILE_NAME> <SAVE_NAME>");
        return 0;
    }
    char *IP=argv[1];
    int PORT=atoi(argv[2]);
    char *fs_name=argv[3];
    char *buffer=argv[4];
    int sockfd; 
    int nsockfd;
    char revbuf[LENGTH]; 
    struct sockaddr_in remote_addr;

/* Get the Socket file descriptor */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",1);
        std::exit(1);
    }

/* Fill the socket address struct */
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(PORT); 
    inet_pton(AF_INET, IP, &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);

/* Try to connect the remote */
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",2);
        exit(1);
    }
    else 
        printf("[Client] Connected to server at port %d...ok!\n", PORT);

/* Send File to Server */
    char sdbuf[LENGTH]; 
    int n = write(sockfd,buffer, strlen(buffer));
    if(n<0) printf("Error: sending filename");

    printf("[Client] Sending %s to the Server... ", fs_name);
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL){
        printf("ERROR: File %s not found.\n", fs_name);
        exit(1);
    }

    bzero(sdbuf, LENGTH); 
    int fs_block_sz;
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0){
        if(send(sockfd, sdbuf, fs_block_sz, 0) < 0){
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
            break;
        }
        bzero(sdbuf, LENGTH);
    }
    printf("Ok File %s from Client was Sent!\n", fs_name);

    close (sockfd);
    printf("[Client] Connection lost.\n");
    return 0;
}