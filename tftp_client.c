#include "tftp_client.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

void cmd_puttftp(int argc, char *argv[]) {
    char send_buffer[BufferSize] = {0};
    if (argc != 3) {
        write(STDOUT_FILENO, WrongCommandput, strlen(WrongCommandput));
        return;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(argv[1], Port, &hints, &res) != 0) {
        perror("getaddrinfo failed");
        return;
    }

    int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return;
    }

    sendWRQ(&sd, res, argv[2]);

    if (recvACK(&sd, res) == 0) {
        FILE *file = fopen(argv[2], "rb");
        if (!file) {
            perror("Failed to open file");
            close(sd);
            freeaddrinfo(res);
            return;
        }

        int packet_number = 1;
        size_t bytes_read;
        while ((bytes_read = fread(send_buffer, 1, BufferSize - 4, file)) > 0) {
            sendData(&sd, res, send_buffer, packet_number++);
            if (recvACK(&sd, res) != 0) {
                perror("Failed to receive ACK");
                break;
            }
        }
        fclose(file);
    }

    close(sd);
    freeaddrinfo(res);
}

void cmd_gettftp(int argc, char *argv[]) {
    char recv_buffer[BufferSize] = {0};

    if (argc != 3) {
        write(STDOUT_FILENO, WrongCommandget, strlen(WrongCommandget));
        return;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(argv[1], Port, &hints, &res) != 0) {
        perror("getaddrinfo failed");
        return;
    }

    int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return;
    }

    // Envoie de la requête RRQ
    sendRRQ(&sd, res, argv[2]);

    FILE *file = fopen(argv[2], "wb");
    if (!file) {
        perror("Failed to open file for writing");
        close(sd);
        freeaddrinfo(res);
        return;
    }

    struct sockaddr_storage server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    int block_number = 1;
    while (1) {
        // Réception du paquet
        int bytes_received = recvfrom(sd, recv_buffer, BufferSize, 0,
                                      (struct sockaddr *)&server_addr, &server_addr_len);
        if (bytes_received < 4 || recv_buffer[1] != 3) { // Paquet incorrect
            perror("Error receiving data");
            break;
        }

        // Écriture des données reçues
        fwrite(recv_buffer + 4, 1, bytes_received - 4, file);

        // Envoie de l'ACK
        sendACK(&sd, (struct sockaddr *)&server_addr, server_addr_len, block_number++);

        // Vérifie la fin de la transmission
        if (bytes_received < BufferSize) break;  // Dernier paquet reçu
    }

    fclose(file);
    close(sd);
    freeaddrinfo(res);
}


void sendWRQ(int *sd, struct addrinfo *res, const char *file) {
    char WRQBuf[BufferSize] = {0};
    WRQBuf[1] = 2;
    sprintf(WRQBuf + 2, "%s", file);
    WRQBuf[strlen(file) + 3] = 0;
    sprintf(WRQBuf + strlen(file) + 4, "netascii");
    sendto(*sd, WRQBuf, strlen(file) + 12, 0, res->ai_addr, res->ai_addrlen);
}

void sendRRQ(int *sd, struct addrinfo *res, const char *file) {
    char RRQBuf[BufferSize] = {0};

    // Consstruct a RQQ request
    RRQBuf[0] = 0;  // Opcode = 1 (RRQ)
    RRQBuf[1] = 1;

    int offset = 2;
    strcpy(RRQBuf + offset, file);
    offset += strlen(file);
    RRQBuf[offset++] = 0;

    // Add "octet" mode
    strcpy(RRQBuf + offset, "octet");
    offset += strlen("octet");
    RRQBuf[offset++] = 0;  // Séparateur NULL

    // Envoyer la requête au serveur
    sendto(*sd, RRQBuf, offset, 0, res->ai_addr, res->ai_addrlen);
}




int recvACK(int *sd, struct addrinfo *res) {
    char RecvBuf[BufferSize] = {0};
    socklen_t addr_len = res->ai_addrlen;
    int bytes_received = recvfrom(*sd, RecvBuf, BufferSize, 0, res->ai_addr, &addr_len);
    return (bytes_received >= 4 && RecvBuf[1] == 4) ? 0 : -1;
}

int recvData(int *sd, struct sockaddr *server_addr, socklen_t *addr_len, char *buffer) {
    int bytes_received = recvfrom(*sd, buffer, BufferSize, 0, server_addr, addr_len);

    if (bytes_received >= 4 && buffer[1] == 3) {  // Opcode 3 pour DATA
        return bytes_received;
    }
    return -1;
}


void sendData(int *sd, struct addrinfo *res, const char *data, int packet_number) {
    char SendBuf[BufferSize] = {0};
    SendBuf[1] = 3;
    SendBuf[2] = (packet_number >> 8) & 0xFF;
    SendBuf[3] = packet_number & 0xFF;
    memcpy(SendBuf + 4, data, BufferSize - 4);
    sendto(*sd, SendBuf, BufferSize, 0, res->ai_addr, res->ai_addrlen);
}
void sendACK(int *sd, struct sockaddr *server_addr, socklen_t addr_len, int block_number) {
    char ack_buffer[4] = {0};
    ack_buffer[1] = 4;  // OPCODE 4 : ACK
    ack_buffer[2] = (block_number >> 8) & 0xFF;  // Bloc numéro MSB
    ack_buffer[3] = block_number & 0xFF;        // Bloc numéro LSB

    sendto(*sd, ack_buffer, sizeof(ack_buffer), 0, server_addr, addr_len);
}



