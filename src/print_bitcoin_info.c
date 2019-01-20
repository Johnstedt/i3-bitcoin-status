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

char* get_sock(){

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd;

    /* first what are we going to send and where are we going to send it? */
    int portno =            80;
    char *host =            "api.coindesk.com";

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fprintf(stderr,"ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL)
        fprintf(stderr, "ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        fprintf(stderr, "ERROR connecting");


    char *header = "GET /v2/bpi/currentprice.json HTTP/1.1\r\nHost: api.coindesk.com\r\n\r\n";
    send(sockfd,header,strlen(header),0);

    char buf[4096];

    ssize_t byte_count = recv(sockfd,buf,sizeof(buf)-1,0); // <-- -1 to leave room for a null terminator
    buf[byte_count] = 0; // <-- add the null terminator
    //printf("recv()'d %d bytes of data in buf\n",(int)byte_count);
    //printf("%s",buf);

    close(sockfd);

    char* ret;
    ret = calloc(1, strlen(buf)+1);

    memcpy(ret, buf, strlen(buf));
    return ret;

}


char* get_price(){

    const char *walk;
    char* price = calloc(1, 10);

    int i = 0;

    for (walk = get_sock(); *walk != '\0'; walk++) {

        if (BEGINS_WITH(walk + 1, "rate_float")) {
            while (!BEGINS_WITH(walk + 12, "}")) {

                walk++;
                i++;
            }
            if(BEGINS_WITH(walk + 12, "}")){
                sprintf(price, "%.*s",i-1, walk + 12 - i + 1);
            }

            break;
        }
    }

    return price;

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

void print_bitcoin_price(yajl_gen json_gen, char *buffer, const char *format_up, const char *format_down) {

    const char *walk;
    char *addr_string = get_price();
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

        } else if (BEGINS_WITH(walk + 1, "price")) {
            outwalk += sprintf(outwalk, "%s", addr_string);
            walk += strlen("price");

        } else {
            *(outwalk++) = '%';
        }
    }


    END_COLOR;
    OUTPUT_FULL_TEXT(buffer);
}
