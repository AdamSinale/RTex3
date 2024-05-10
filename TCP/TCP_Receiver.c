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
    if (argc < 5) {                          // if there are inputs missing return error
        printf("Invalid arguments.\nUsage: %s -p <port> -algo <algorithm>\n",argv[0]);
        return 0;
    }

    socklen_t len;
    char buffer[BUFFER_SIZE];
    int port = atoi(argv[2]);
    char *algo;
    if(strcmp(argv[4],"reno") == 0||strcmp(argv[4],"cubic") == 0){  // if not reno/cubic return error
        algo = argv[4];
    } else{ return 1; }
 
    float *timesArr = NULL;                          // list of times for statistics
    timesArr = (float*)malloc(sizeof(float));
    int timesArr_size = 0;
    double *mbArr = NULL;                            // list of mb/s for statistics
    mbArr = (double*)malloc(sizeof(double));
    int mbArr_size = 0;
    int countTimes = 0;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);    // create IPv4 socket 
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;       // add address family for IPv4 socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { //bind socket to address
        perror("Bind failed");
        return 1;
    }
    if (listen(server_sock, 1) < 0) {  // allow listening to 1 client
        perror("Listen failed");
        return 1;
    }
    printf("Waiting for TCP connection...\n");
    len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len); // accept clients socket
    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    } 
    printf("Sender connected...\n");
    
    if (setsockopt(client_sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0) {  // set algo reno/cubic
        perror("setsockopt");
        return 1;
    }
    printf("Set algo to %s, beginning to receive file...\n", algo);

    time_t start, end;
    float time_taken;

    while(1){
        size_t total_bytes = 0;
        start = clock();  // start timer
        ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        total_bytes += bytes_received;

        if (bytes_received == -1) {
            perror("recv");
            return 1;
        }
        if (strncmp(buffer, "exit", 4) == 0) {  // if exit message then leave
            printf("exit message received\n");
            break;
        }

        while (total_bytes < sizeof(buffer)) {  // receive data piece by piece
            bytes_received = recv(client_sock, buffer + total_bytes, sizeof(buffer) - total_bytes, 0);
            if (bytes_received == -1) { 
                perror("recv");
                return 1;
            }
            if (bytes_received == 0) {
                printf("sender closed the connection\n");
                break;
            }
            total_bytes += bytes_received;
        }
        end = clock(); // close timer
        printf("File transfer completed.\n");
        time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;  // time in seconds
        timesArr = (float*)realloc(timesArr,(timesArr_size+1)*sizeof(float));
        timesArr[timesArr_size++] = time_taken * 1000;      // add to list of times and time in ms
        mbArr = (double*)realloc(mbArr,(mbArr_size+1)*sizeof(double));
        mbArr[mbArr_size++] = 2/(time_taken);               // we always send 2mb so 2mb/seconds taken to send
        countTimes++; // count number of sends
    }
    
    printf("----------------------------------\n");
    printf("STATISTICS :\n");
    float totalTime=0;
    float totalMS=0;
    for(int i=0; i<countTimes; i++){
        printf("Run #%d Data: Time=%fms; Speed=%fMB/s\n", i+1, timesArr[i], mbArr[i]); //print send stats
        totalTime += timesArr[i];  // sum of ms
        totalMS += mbArr[i];  // sum of mb/s
    }
    printf("\nAverage time: %fms.\n", (totalTime/countTimes));    // average ms for send
    printf("\nAverage bandwidth:: %fmb.\n", (totalMS/countTimes)); // average mb/s for send
    printf("----------------------------------\n");
    printf("Receiver end.");
    close(client_sock); // close connection
    close(server_sock);

    return 0;
}