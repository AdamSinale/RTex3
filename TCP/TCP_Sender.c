#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 65536*2
#define DATA_SIZE 2*1024*1024

char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
    return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
    return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
    *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 7) {        // if there are inputs missing return error
        printf("Send: %s -ip <IP> -p <port> -algo <algorithm>\n", argv[0]);
        return 1;
    }
    char *ip = argv[2];
    int port = atoi(argv[4]);
    char *algo;
    if(strcmp(argv[6],"reno") == 0||strcmp(argv[6],"cubic") == 0){  // if not reno/cubic return error
        algo = argv[6];
    } else{ return 1; }

    char* data = util_generate_random_data(DATA_SIZE);   // generate random data

    int sock = socket(AF_INET, SOCK_STREAM, 0);    // create IPv4 socket 
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in addr;                  // add address family for IPv4 socket
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {   // connect to server
        perror("Connection failed");
        return 1;
    } else{ printf("connected to server\n"); }

    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0) {  // set algo reno/cubic
        perror("setsockopt");
        return 1;
    }

    while (1) {
        size_t bytes_sent = 0;
        while (bytes_sent < DATA_SIZE) {            // while not all data sent
            ssize_t sent = send(sock, data + bytes_sent, DATA_SIZE - bytes_sent, 0);   // send next chunk of data
            if (sent < 0) {
                perror("ERROR writing to socket");
            }
            bytes_sent += sent;     // add to the total bytes sent
        }
        printf("File sent successfully.\n");
        
        char choice;
        printf("Do you want to send the file again? (y/n): ");    // send again?
        scanf(" %c", &choice);
        if (choice != 'y') {
            break;
        }
    }

    send(sock, "exit", strlen("exit"), 0);    // if said no then send exit

    close(sock); // close connection

    return 0;
}