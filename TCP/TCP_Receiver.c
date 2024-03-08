#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 2*1024*1024

int main(int argc, char *argv[]) {
    printf("Starting Receiver...\n");
    if (argc < 5) {
        printf("Invalid arguments.\nUsage: %s -p <port> -algo <algorithm>\n",argv[0]);
        return 0;
    }

    socklen_t len;
    char buffer[BUFFER_SIZE];
    int port = atoi(argv[2]);
    char *algo;
    if(strcmp(argv[4],"reno") == 0||strcmp(argv[4],"cubic") == 0){
        algo = argv[4];
    } else{ return 1; }

    float *timesArr = NULL;
    timesArr = (float*)malloc(sizeof(float));
    int timesArr_size = 0;
    double *mbArr = NULL;
    mbArr = (double*)malloc(sizeof(double));
    int mbArr_size = 0;
    int countTimes = 0;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    if (listen(server_sock, 1) < 0) {
        perror("Listen failed");
        return 1;
    }
    printf("Waiting for TCP connection...\n");
    len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len);
    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    } 
    printf("Sender connected...\n");
    
    if (setsockopt(client_sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0) {
        perror("setsockopt");
        return 1;
    }
    printf("Set algo to %s, beginning to receive file...\n", algo);

    time_t start, end;
    float time_taken;

    // while (1) {
    //     size_t total_bytes = 0;  // the total bytes received so far
    //     start = clock();  // start measuring the time
    //     ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
    //     total_bytes += bytes_received;
    //     if (bytes_received == -1) {
    //         perror("recv");
    //         return 1;
    //     }
    //     if (strncmp(buffer, "exit", 4) == 0) {
    //         printf("exit message received\n");
    //         break;
    //     }
    //     while (total_bytes < sizeof(buffer)) {
    //         bytes_received = recv(client_sock, buffer + total_bytes, sizeof(buffer) - total_bytes, 0);
    //         if (bytes_received == -1) {
    //             perror("recv");
    //             return 1;
    //         }
    //         if (bytes_received == 0) {
    //             printf("sender closed the connection\n");
    //             break;
    //         }
    //         total_bytes += bytes_received;
    //     }
    //     end = clock();
    //     printf("File transfer completed.\n");
    //     time_taken = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;
    //     timesArr = (float*)realloc(timesArr,(timesArr_size+1)*sizeof(float));
    //     timesArr[timesArr_size++] = time_taken;
    //     mbArr = (double*)realloc(mbArr,(mbArr_size+1)*sizeof(double));
    //     mbArr[mbArr_size++] = (bytes_received/1024.0)/(time_taken/1000);
    //     countTimes++;
    // }
    while(1){
        size_t total_bytes = 0;  // the total bytes received so far
        start = clock();  // start measuring the time
        ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        total_bytes += bytes_received;

        if (bytes_received == -1) {  // check for errors
            perror("recv");
            return 1;
        }
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("exit message received\n");
            break;
        }
        // keep receiving until the file is received
        while (total_bytes < sizeof(buffer)) {
            bytes_received = recv(client_sock, buffer + total_bytes, sizeof(buffer) - total_bytes, 0);
            if (bytes_received == -1) {  // check for errors
                perror("recv");
                return 1;
            }
            if (bytes_received == 0) {
                printf("sender closed the connection\n");
                break;
            }
            total_bytes += bytes_received;
        }
        end = clock();
        printf("File transfer completed.\n");
        time_taken = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;
        timesArr = (float*)realloc(timesArr,(timesArr_size+1)*sizeof(float));
        timesArr[timesArr_size++] = time_taken;
        mbArr = (double*)realloc(mbArr,(mbArr_size+1)*sizeof(double));
        mbArr[mbArr_size++] = (bytes_received/1024.0)/(time_taken/1000);
        countTimes++;
    }
    
    printf("----------------------------------");
    printf("STATISTICS :\n");
    float totalTime=0;
    float totalMS=0;
    for(int i=0; i<countTimes; i++){
        printf("Run #%d Data: Time=%fms; Speed=%fMB/s\n", i+1, timesArr[i], mbArr[i]);
        totalTime += timesArr[i];
        totalMS += mbArr[i];
    }
    printf("\nAverage time: %fms.\n", (totalTime/countTimes));
    printf("\nAverage bandwidth:: %fmb.\n", (totalMS/countTimes));
    printf("----------------------------------");
    printf("Receiver end.");
    close(client_sock);
    close(server_sock);

    return 0;
}