
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

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

    // Envoie de la requête d'écriture (WRQ)
    sendWRQ(&sd, res, argv[2]);

    FILE *file = fopen(argv[2], "rb");
    if (!file) {
        perror("Failed to open file");
        close(sd);
        freeaddrinfo(res);
        return;
    }

    char buffer[BufferSize];
    int packet_number = 1;
    size_t bytes_read;

    // Envoi des paquets de données
    while ((bytes_read = fread(buffer, 1, BufferSize - 4, file)) > 0) {
        sendData(&sd, res, buffer, packet_number++);
        if (recvACK(&sd, res) != 0) {
            perror("Failed to receive ACK");
            break;
        }
    }

    fclose(file);
    close(sd);
    freeaddrinfo(res);
}
