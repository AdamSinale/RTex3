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

void receive_file(int sockfd) {
    FILE *file;
    char buffer[BUFFER_SIZE];
    time_t start, end;
    double time_taken;
    int bytesReceived;

    file = fopen("received_file.txt", "w");
    if (file == NULL) {
        perror("Error creating file");
        return;
    }

    start = time(NULL);

    while ((bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
        if (bytesReceived < BUFFER_SIZE)
            break;
    }
    fclose(file);

    end = time(NULL);
    time_taken = difftime(end, start);

    printf("File received successfully. Time taken: %.2f seconds\n", time_taken);

    fclose(file);
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    } else{ printf("Socket creation successful\n"); }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        return 1;
    } else{ printf("Bind successful\n"); }

    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        return 1;
    } else{ printf("Listen successful\n"); }

    len = sizeof(cliaddr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
    if (newsockfd < 0) {
        perror("Accept failed");
        return 1;
    } else{ printf("Accept successful\n"); }

    receive_file(newsockfd);
    // return new file?
    
    while (1) {
        recv(newsockfd, buffer, BUFFER_SIZE, 0);
        if (strcmp(buffer, "exit") == 0) {
            break;
        } else {
            receive_file(newsockfd);
        }
    }

    close(newsockfd);
    close(sockfd);

    return 0;
}
