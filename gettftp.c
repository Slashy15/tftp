// Improved gettftp.c with debugging messages
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

void cmd_gettftp(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./tftp gettftp <host> <file>\n");
        return;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // Resolve the server address
    if (getaddrinfo(argv[1], "1069", &hints, &res) != 0) {
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

    // Send the RRQ request
    sendRRQ(&sd, res, argv[2]);
    printf("[DEBUG] RRQ sent for file: %s\n", argv[2]);

    FILE *file = fopen(argv[2], "wb");
    if (!file) {
        perror("[ERROR] Failed to open file for writing");
        close(sd);
        freeaddrinfo(res);
        return;
    }

    struct sockaddr_storage server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    int block_number = 1;

    while (1) {
        char recv_buffer[BufferSize];

        printf("[DEBUG] Waiting for data from the server...\n");

        // Receive data from the server
        ssize_t bytes_received = recvfrom(sd, recv_buffer, BufferSize, 0,
                                          (struct sockaddr *)&server_addr, &server_addr_len);

        if (bytes_received < 0) {
            perror("[ERROR] Receiving data failed");
            break;
        }

        printf("[DEBUG] Received %zd bytes from server\n", bytes_received);

        if (bytes_received < 4 || recv_buffer[1] != 3) {
            fprintf(stderr, "[ERROR] Invalid DATA packet received\n");
            break;
        }

        // Write received data to the file
        fwrite(recv_buffer + 4, 1, bytes_received - 4, file);
        printf("[DEBUG] Written %zd bytes to file\n", bytes_received - 4);

        // Send an ACK back to the server
        sendACK(&sd, (struct sockaddr *)&server_addr, server_addr_len, block_number++);
        printf("[DEBUG] Sent ACK for block %d\n", block_number - 1);

        // If the last packet is smaller than BufferSize, transfer is complete
        if (bytes_received < BufferSize) {
            printf("[INFO] File transfer complete\n");
            break;
        }
    }

    fclose(file);
    close(sd);
    freeaddrinfo(res);
}





