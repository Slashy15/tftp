//
// Created by alex on 12/18/24.
//
#ifndef UTILS_H
#define UTILS_H
#include <netdb.h>
#define BufferSize 512
#define Port "1069"
#define WrongCommandget "Please enter a command in the following format : gettftp <host> <file>\r\n"
#define WrongCommandput "Please enter a command in the following format : puttftp <host> <file>\r\n"


void sendWRQ(int *sd, struct addrinfo *res, const char *file);
void sendRRQ(int *sd, struct addrinfo *res, const char *file);
int recvACK(int *sd, struct addrinfo *res);
int recvData(int *sd, struct sockaddr *server_addr, socklen_t *addr_len, char *buffer);
void sendData(int *sd, struct addrinfo *res, const char *data, int packet_number);
void sendACK(int *sd, struct sockaddr *server_addr, socklen_t addr_len, int block_number);
#endif //UTILS_H
