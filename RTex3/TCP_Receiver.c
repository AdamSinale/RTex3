#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

float receive_file(int server_sock) {
    FILE *file;
    char buffer[BUFFER_SIZE];
    time_t start, end;
    float time_taken;
    int bytesReceived;

    file = fopen("received_file.txt", "w");
    if (file == NULL) {
        perror("Error creating file");
        return 0;
    }

    start = clock();

    while ((bytesReceived = recv(server_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
        if (bytesReceived < BUFFER_SIZE)
            break;
    }
    fclose(file);

    end = clock();
    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;

    printf("File transfer completed.\n");
    return time_taken;
}

int main() {
    printf("Starting Receiver...\n");

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;
    char buffer[BUFFER_SIZE];
    float *timesArr = NULL;
    timesArr = (float*)malloc(sizeof(float));
    int timesArr_size = 0;
    double *mbArr = NULL;
    mbArr = (double*)malloc(sizeof(double));
    int mbArr_size = 0;
    int countTimes = 0;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        return 1;
    }
    printf("Waiting for TCP connection...\n");

    len = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len);
    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    } 
    printf("Sender connected, beginning to receive file...\n");
    while (1) {
        int bytesReceived = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived < 0) {
            perror("Receive error");
            break;
        } else if (bytesReceived == 0) {
            printf("Sender closed the connection.\n");
            break;
        } else {
            buffer[bytesReceived] = '\0';
            if (strcmp(buffer, "exit") == 0) {
                printf("Sender sent exit message.\n");
                break;
            } else {
                timesArr = (float*)realloc(timesArr,(timesArr_size+1)*sizeof(float));
                timesArr[timesArr_size++] = receive_file(client_sock);
                mbArr = (double*)realloc(mbArr,(mbArr_size+1)*sizeof(double));
                mbArr[mbArr_size++] = bytesReceived / 1024.0;
                countTimes++;
            }
        }
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
    printf("\nAverage time: %fms.\n", totalTime/countTimes);
    printf("\nAverage bandwidth:: %fmb.\n", totalMS/countTimes);
    printf("----------------------------------");
    printf("Receiver end.");
    close(client_sock);
    close(server_sock);

    return 0;
}
