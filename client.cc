#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define LENGTH 512

struct sockaddr_in addr_remote;

int main(int argc, char *argv[]){
    if(argc<3){
        puts("Usage: ./client <IP> <PORT>");
        puts("Commands: send <SRC_FILE> <DST_FILE>");
        return 0;
    }
    char *IP=argv[1];
    int PORT=atoi(argv[2]);
    int sockfd;
    char revbuf[LENGTH]; 
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",1);
        exit(1);
    }

    addr_remote.sin_family = AF_INET; 
    addr_remote.sin_port = htons(PORT); 
    inet_pton(AF_INET, IP, &addr_remote.sin_addr); 
    bzero(&(addr_remote.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&addr_remote, sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",2);
        exit(1);
    }

    while(1){
        char sdbuf[LENGTH];
        char name_r[256], name_w[256], cmd[16], *word;
        printf(">>> ");
        scanf("%s", cmd);
        if(cmd[0]=='e'){
            write(sockfd, "\0", 1);
            close(sockfd);
            printf("[Client] Connection lost.\n");
            exit(1);
        }
        else{
            scanf("%s %s", name_r, name_w);
            int n = write(sockfd, name_w, strlen(name_w));
            if(n<0) printf("Error: sending file");

            printf("[Client] Sending %s to Server", name_r);
            FILE *fs = fopen(name_r, "rb");
            if(fs == NULL){
                printf("\nERROR: File %s not found.\n", name_r);
                exit(1);
            }

            int fs_block_sz, i=0;
            while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0){
                if(i%100 == 0) printf(".");
                if(send(sockfd, sdbuf, fs_block_sz, 0) < 0){
                    fprintf(stderr, "\nERROR: Failed to send file %s. (errno = %d)\n", name_r, errno);
                    break;
                }
                i++;
            }
            printf("\n[Client] File %s was sent sucessfully.\n", name_r);
            fclose(fs);
        }
    }
    return 0;
}