// puttftp.c with detailed debugging and correct port handling
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

// Function to handle the TFTP PUT command with detailed debugging
void cmd_puttftp(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./tftp puttftp <host> <file>\n");
        return;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // Resolve the server address
    if (getaddrinfo(argv[1], Port, &hints, &res) != 0) {
        perror("[ERROR] getaddrinfo failed");
        return;
    }

    // Create a UDP socket
    int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0) {
        perror("[ERROR] Socket creation failed");
        freeaddrinfo(res);
        return;
    }

    printf("[DEBUG] Socket created successfully\n");

    // Send a Write Request (WRQ) to the server
    sendWRQ(&sd, res, argv[2]);
    printf("[DEBUG] WRQ sent for file: %s\n", argv[2]);

    FILE *file = fopen(argv[2], "rb");
    if (!file) {
        perror("[ERROR] Failed to open file for reading");
        close(sd);
        freeaddrinfo(res);
        return;
    }

    struct sockaddr_storage server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    int packet_number = 1;
    char buffer[BufferSize];
    ssize_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BufferSize - 4, file)) > 0) {
        printf("[DEBUG] Read %zd bytes from file\n", bytes_read);

        // Send data packet
        sendData(&sd, res, buffer, packet_number++);
        printf("[DEBUG] Sent DATA packet number %d with content: %.*s\n", packet_number - 1, (int)bytes_read, buffer);

        // Receive ACK from server
        ssize_t ack_received = recvfrom(sd, buffer, BufferSize, 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (ack_received < 0) {
            perror("[ERROR] Receiving ACK failed");
            break;
        }

        // Check if it's a valid ACK
        if (buffer[1] != 4) {
            fprintf(stderr, "[ERROR] Invalid ACK received\n");
            break;
        }

        printf("[DEBUG] Received valid ACK for packet %d\n", packet_number - 1);
    }

    fclose(file);
    close(sd);
    freeaddrinfo(res);
    printf("[INFO] File transfer completed\n");
}

