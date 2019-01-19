// vim:ts=4:sw=4:expandtab
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>

#include "i3status.h"


char* get_height() {
    FILE *fp;
    char path[1035];
    char *height;

    /* Open the command for reading. */
    fp = popen("bitcoin-cli -rpcuser=admin -rpcpassword=admin getblockcount", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    fgets(path, sizeof(path)-1, fp);

    height = calloc(1, strlen(path));

    memcpy(height, path, strlen(path)-1);

    /* close */
    pclose(fp);

    return height;
}


void print_bitcoin_info(yajl_gen json_gen, char *buffer, const char *format_up, const char *format_down) {
    const char *walk;
    char *addr_string = get_height();
    char *outwalk = buffer;

    if (addr_string == NULL) {
        START_COLOR("color_bad");
        outwalk += sprintf(outwalk, "%s", format_down);
        END_COLOR;
        OUTPUT_FULL_TEXT(buffer);
        return;
    }

    START_COLOR("color_good");
    for (walk = format_up; *walk != '\0'; walk++) {
        if (*walk != '%') {
            *(outwalk++) = *walk;

        } else if (BEGINS_WITH(walk + 1, "ip")) {
            outwalk += sprintf(outwalk, "%s", addr_string);
            walk += strlen("ip");

        } else {
            *(outwalk++) = '%';
        }
    }


    END_COLOR;
    OUTPUT_FULL_TEXT(buffer);
}
