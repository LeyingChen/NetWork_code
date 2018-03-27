/* worker application */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

typedef struct bmessage {
    int length;
    char fpath[20];
    int spos;
    int epos;
} bmessage_t;

int main(int argc, const char *argv[])
{
    int s, cs;
    struct sockaddr_in master, worker;

    // Create socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create socket");
    return -1;
    }
    printf("Socket created");

    // Prepare the sockaddr_in structure
    worker.sin_family = AF_INET;
    worker.sin_addr.s_addr = INADDR_ANY;
    worker.sin_port = htons(12345);

    // Bind
    if (bind(s,(struct sockaddr *)&worker, sizeof(worker)) < 0) {
        perror("bind failed. Error");
        return -1;
    }
    printf("bind done");

    // Listen
    listen(s, 3);

    // Accept and incoming connection
    printf("Waiting for incoming connections...");

    // accept connection from an incoming client
    int c = sizeof(struct sockaddr_in);
    if ((cs = accept(s, (struct sockaddr *)&master, (socklen_t *)&c)) < 0) {
        perror("accept failed");
        return 1;
    }
    printf("Connection accepted");

    bmessage_t *mp = (bmessage_t *)malloc(sizeof(bmessage_t));
    if (recv(cs, mp, sizeof(bmessage_t), 0) < 0) {
        printf("recv failed");
    }

    FILE *book;
    int char_count[26];
    for(int i=0; i<26; i++) char_count[i] = 0;
    if((book = fopen(mp->fpath, "r")) == NULL){
        printf("Book file open failed!\n");
        return 0;
    }

    fseek(book, ntohl(mp->spos), SEEK_SET);
    while(ftell(book) <= ntohl(mp->epos)){
        char c;
        c = fgetc(book);
        if(isalpha(c)){
            if(isupper(c)) c = tolower(c);
            char_count[c-'a']++;
        }
    }

    for(int i=0; i<26; i++) char_count[i] = htonl(char_count[i]);
    if (send(cs, char_count, 104, 0) < 0) {
        printf("Send failed");
        return 1;
    }

    return 0;
}
