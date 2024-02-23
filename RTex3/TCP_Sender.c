#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    FILE *file;
    char *filename = "file.txt";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in recAddr;
    memset(&recAddr, 0, sizeof(recAddr));

    recAddr.sin_family = AF_INET;
    recAddr.sin_port = htons(PORT);
    recAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&recAddr, sizeof(recAddr)) < 0) {
        perror("Connection failed");
        return 1;
    } else{ printf("connected to server\n"); }

    while (1) {
        file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }
        fprintf(file, "1\n19\nGrace Hopper coined \"bug\" for computer glitches. In 1947, she found a moth causing issues in Harvard's Mark II.\n3\n4\n13\n9\n5\n3\n6\n5\n5\n2\n5\ncomputer\n3\n10\n3\n0");
        fclose(file);

        file = fopen(filename, "r");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }

        while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
            send(sockfd, buffer, strlen(buffer), 0);
        }
        fclose(file);

        printf("File sent successfully.\n");
        
        char choice;
        printf("Do you want to send the file again? (y/n): ");
        scanf(" %c", &choice);
        if (choice != 'y') {
            break;
        }
    }

    send(sockfd, "exit", strlen("exit"), 0);

    close(sockfd);

    return 0;
}
