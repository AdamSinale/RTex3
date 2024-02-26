#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define MAX_BUFFER_SIZE 1024
#define TIMEOUT_SEC 2
#define SYN_FLAG 0x01
#define ACK_FLAG 0x02

// Structure to represent RUDP packet header
typedef struct {
    uint16_t length;
    uint16_t checksum;
    uint8_t flags;
} RUDPHeader;

// Function to create a RUDP socket and perform handshake
int rudp_socket(struct sockaddr *addr, socklen_t addrlen) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Perform the handshake
    // Send SYN packet
    RUDPHeader syn_packet;
    syn_packet.length = 0;
    syn_packet.checksum = 0; // Placeholder for checksum
    syn_packet.flags = SYN_FLAG;
    sendto(sockfd, &syn_packet, sizeof(RUDPHeader), 0, addr, addrlen);

    // Wait for ACK packet
    RUDPHeader ack_packet;
    if (recvfrom(sockfd, &ack_packet, sizeof(RUDPHeader), 0, NULL, NULL) < 0) {
        perror("Error receiving ACK packet");
        exit(EXIT_FAILURE);
    }

    // Verify ACK packet
    if (ack_packet.flags != ACK_FLAG) {
        printf("Error: Handshake failed.\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Sends data to the peer with acknowledgment and retransmission
void rudp_send(int sockfd, struct sockaddr *addr, socklen_t addrlen, const void *data, size_t length) {
    ssize_t bytes_sent;
    char buffer[MAX_BUFFER_SIZE];
    RUDPHeader header;
    int ack_received = 0;

    while (!ack_received) {
        // Construct RUDP packet
        header.length = htons(length);
        // Calculate checksum (not implemented in this example)
        header.checksum = 0; // Placeholder for checksum
        header.flags = 0; // Placeholder for flags
        memcpy(buffer, &header, sizeof(RUDPHeader));
        memcpy(buffer + sizeof(RUDPHeader), data, length);

        // Send packet
        bytes_sent = sendto(sockfd, buffer, sizeof(RUDPHeader) + length, 0, addr, addrlen);
        if (bytes_sent < 0) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }

        // Wait for acknowledgment
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        char ack_buffer[sizeof(RUDPHeader)];
        ssize_t bytes_received = recvfrom(sockfd, ack_buffer, sizeof(RUDPHeader), 0, NULL, NULL);
        if (bytes_received < 0) {
            continue;
        }

        // Check acknowledgment packet
        RUDPHeader *ack_header = (RUDPHeader *)ack_buffer;
        if (ack_header->flags == ACK_FLAG) {
            ack_received = 1;
        }
    }
}

// Function to receive data from a peer
ssize_t rudp_receive(int sockfd, void *buffer, size_t length, struct sockaddr *addr, socklen_t *addrlen) {
    ssize_t bytes_received;
    bytes_received = recvfrom(sockfd, buffer, length, 0, addr, addrlen);
    if (bytes_received < 0) {
        perror("Error receiving data");
        exit(EXIT_FAILURE);
    }
    return bytes_received;
}

// closes a connection between peers
void rudp_close(int sockfd) {
    close(sockfd);
}
