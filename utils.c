#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

// Sends a Write Request (WRQ) to the TFTP server
void sendWRQ(int *sd, struct addrinfo *res, const char *file) {
    char WRQBuf[BufferSize] = {0};
    WRQBuf[1] = 2;  // Opcode for WRQ
    sprintf(WRQBuf + 2, "%s", file);  // Add the file name
    WRQBuf[strlen(file) + 3] = 0;  // Null terminator
    sprintf(WRQBuf + strlen(file) + 4, "netascii");  // Transfer mode
    sendto(*sd, WRQBuf, strlen(file) + 12, 0, res->ai_addr, res->ai_addrlen);  // Send WRQ
}

// Sends a Read Request (RRQ) to the TFTP server
void sendRRQ(int *sd, struct addrinfo *res, const char *file) {
    char RRQBuf[BufferSize] = {0};

    // Construct an RRQ request
    RRQBuf[0] = 0;  // Opcode for RRQ
    RRQBuf[1] = 1;

    int offset = 2;
    strcpy(RRQBuf + offset, file);  // Add the file name
    offset += strlen(file);
    RRQBuf[offset++] = 0;  // Null terminator

    // Add "octet" mode for binary transfer
    strcpy(RRQBuf + offset, "octet");
    offset += strlen("octet");
    RRQBuf[offset++] = 0;  // Null terminator

    // Send the request to the server
    sendto(*sd, RRQBuf, offset, 0, res->ai_addr, res->ai_addrlen);
}

// Receives an ACK packet from the TFTP server
int recvACK(int *sd, struct addrinfo *res) {
    char RecvBuf[BufferSize] = {0};
    socklen_t addr_len = res->ai_addrlen;
    int bytes_received = recvfrom(*sd, RecvBuf, BufferSize, 0, res->ai_addr, &addr_len);

    // Check if the packet is a valid ACK with opcode 4
    return (bytes_received >= 4 && RecvBuf[1] == 4) ? 0 : -1;
}

// Receives data packets from the TFTP server
int recvData(int *sd, struct sockaddr *server_addr, socklen_t *addr_len, char *buffer) {
    int bytes_received = recvfrom(*sd, buffer, BufferSize, 0, server_addr, addr_len);

    // Check if the packet contains valid data (opcode 3)
    if (bytes_received >= 4 && buffer[1] == 3) {
        return bytes_received;
    }
    return -1;
}

// Sends a data packet to the TFTP server
void sendData(int *sd, struct addrinfo *res, const char *data, int packet_number) {
    char SendBuf[BufferSize] = {0};
    SendBuf[1] = 3;  // Opcode for DATA
    SendBuf[2] = (packet_number >> 8) & 0xFF;  // Block number MSB
    SendBuf[3] = packet_number & 0xFF;  // Block number LSB
    memcpy(SendBuf + 4, data, BufferSize - 4);  // Add the file data
    sendto(*sd, SendBuf, BufferSize, 0, res->ai_addr, res->ai_addrlen);  // Send the packet
}

// Sends an ACK packet to the TFTP server
void sendACK(int *sd, struct sockaddr *server_addr, socklen_t addr_len, int block_number) {
    char ack_buffer[4] = {0};
    ack_buffer[1] = 4;  // Opcode for ACK
    ack_buffer[2] = (block_number >> 8) & 0xFF;  // Block number MSB
    ack_buffer[3] = block_number & 0xFF;  // Block number LSB

    // Send the ACK back to the server
    sendto(*sd, ack_buffer, sizeof(ack_buffer), 0, server_addr, addr_len);
}
