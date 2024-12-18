#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gettftp.h"
#include "utils.h"
#include "puttftp.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Please use this command: ./tftp <command> <host> <file>\n");
        return EXIT_FAILURE;
    }

    printf("Received command: %s\n", argv[1]);  // Debug message

    if (strcmp(argv[1], "puttftp") == 0) {
        cmd_puttftp(argc - 1, &argv[1]);
    } else if (strcmp(argv[1], "gettftp") == 0) {
        cmd_gettftp(argc - 1, &argv[1]);
    } else {
        fprintf(stderr, "Invalid command. Use gettftp or puttftp.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
