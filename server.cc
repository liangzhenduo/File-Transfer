#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <resolv.h>
#include <pthread.h>

#define BACKLOG 5
#define LENGTH 512

struct sockaddr_in addr_local;
struct sockaddr_in addr_remote;
struct tm *timeinfo;
time_t rawtime;
pthread_t thread_id;

void *handle(void *socket_fd){
    int id = *((int *)socket_fd);
    rawtime = time(NULL);
    timeinfo = localtime ( &rawtime );
    printf("[Server] Client_%d connected from %s at %s", id-3, inet_ntoa(addr_remote.sin_addr), asctime(timeinfo));
    
    char name[256];
    char revbuf[LENGTH];
    while(1){
        memset(name, 0, sizeof(name));
        if (read(id, name, 256) == -1)
            printf("ERROR reading from socket");
        if(name[0]=='\0')  //exit命令
            break;
        
        FILE *fr = fopen(name, "a");
        if(fr == NULL)
            printf("File %s Cannot be opened file on server.\n", name);
        else{
            bzero(revbuf, LENGTH); 
            int fr_block_sz = 0;
            
            while((fr_block_sz = recv(id, revbuf, LENGTH, 0)) > 0) {
                int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
                if(write_sz < fr_block_sz)
                    printf("File write failed on server.\n");
                bzero(revbuf, LENGTH);
                if (fr_block_sz == 0 || fr_block_sz != 512) 
                    break;
            }
            
            if(fr_block_sz < 0){
                if (errno == EAGAIN)
                    printf("recv() timed out.\n");
                else
                    fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                    exit(1);
            }
            
            rawtime = time(NULL);
            timeinfo = localtime( &rawtime );
            printf("[Server] File %s received from Client_%d at %s", name, id, asctime(timeinfo));
        }
    }

    rawtime = time(NULL);
    timeinfo = localtime( &rawtime );
    printf("[Server] Client_%d disconnected at %s", id-3, asctime(timeinfo));
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    if(argc<2){
        puts("Usage: ./server <PORT>");
        return 0;
    }

    int PORT=atoi(argv[1]);
    int sockfd, nsockfd;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
        exit(1);
    }
    else{
        time_t rawtime=time(NULL);
        struct tm *timeinfo = localtime( &rawtime );
        printf("[Server] Server starting at %s",asctime(timeinfo));
    }

    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(PORT);
    addr_local.sin_addr.s_addr = INADDR_ANY;
    bzero(&(addr_local.sin_zero), 8);

    if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){  //绑定端口
        fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
        exit(1);
    }
    else 
        printf("[Server] Binded TCP Port %d in 127.0.0.1 sucessfully.\n",PORT);

    if(listen(sockfd, BACKLOG) == -1){  //监听远程连接
        fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
        exit(1);
    }
    else
        printf("[Server] Listening Port %d successfully.\n", PORT);

    
    while(1){
        socklen_t sin_size = sizeof(struct sockaddr_in);

        if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1) {
            fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
            exit(1);
        }

        if(pthread_create(&thread_id, NULL, handle, (void *)(&nsockfd)) == -1){
            fprintf(stderr, "ERROR: pthread_create failed.\n");
            exit(1);
        }        
    }
    return 0;
}