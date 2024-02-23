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
    int sock;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in addr;
    FILE *file;
    char *filename = "file.txt";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
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
            send(sock, buffer, strlen(buffer), 0);
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

    send(sock, "exit", strlen("exit"), 0);

    close(sock);

    return 0;
}
