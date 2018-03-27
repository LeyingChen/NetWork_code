/* master application */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ADDRLEN 16

typedef struct bmessage {
    int length;
    char fpath[20];
    int spos;
    int epos;
} bmessage_t;

int main(int argc, const char *argv[])
{
    FILE *conf;
    char buf[ADDRLEN] = {0};
    char addr[2][ADDRLEN] = {{0},{0}};
    int len;
    int n = 0;
    if((conf = fopen("workers.conf", "r")) == NULL){
        perror("Conference file open failed!\n");
        return 1;
    }
    while(fgets(buf, ADDRLEN-1, conf)){
        len = strlen(buf);
        buf[len-1] = '\0';
        strcpy(addr[n], buf);
        memset(buf, 0, ADDRLEN);
        n++;
    }

    fclose(conf);

    int sock1, sock2;
    struct sockaddr_in worker1, worker2;
    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    if(sock1 == -1){
        perror("Could not create socket\n");
        return 1;
    }
    printf("Socket1 created!\n");

    sock2 = socket(AF_INET, SOCK_STREAM, 0);
    if(sock2 == -1){
        perror("Could not create socket\n");
        return 1;
    }
    printf("Socket1 created!\n");

    worker1.sin_addr.s_addr = inet_addr(addr[0]);
    worker1.sin_family = AF_INET;
    worker1.sin_port = htons(12345);

    worker2.sin_addr.s_addr = inet_addr(addr[1]);
    worker2.sin_family = AF_INET;
    worker2.sin_port = htons(12345);

    if (connect(sock1, (struct sockaddr *)&worker1, sizeof(worker1)) < 0) {
        perror("worker1 connect failed. Error");
        return 1;
    }
    printf("Worker1 connected\n");

    if (connect(sock2, (struct sockaddr *)&worker2, sizeof(worker2)) < 0) {
        perror("worker2 connect failed. Error");
        return 1;
    }
    printf("Worker2 connected\n");

    FILE *book;
    int nchar = 0;
    int mid = 0;
    if(argc != 2){
        printf("Format error!\n");
        return 0;
    }
    if((book = fopen(argv[1], "r")) == NULL){
        printf("Book file open failed!\n");
        return 0;
    }
    while(fgetc(book) != EOF)
        nchar++;

    mid = nchar>>1;
    fclose(book);
    char *file_addr = "war_and_peace.txt";
    int ccount1[26];
    int ccount2[26];
    for(int i=0; i<26; i++){
        ccount1[i] = 0;
        ccount2[i] = 0;
    }

    bmessage_t mes1, mes2;
    mes1.length = htonl(sizeof(bmessage_t));
    strcpy(mes1.fpath, file_addr);
    mes1.spos = htonl(0);
    mes1.epos = htonl(mid);

    mes2.length = htonl(sizeof(bmessage_t));
    strcpy(mes2.fpath, file_addr);
    mes2.spos = htonl(mid+1);
    mes2.epos = htonl(nchar-1);

    if (send(sock1, (void *)&mes1, sizeof(bmessage_t), 0) < 0) {
        printf("Send1 failed");
        return 1;
    }
    if (send(sock2, (void *)&mes2, sizeof(bmessage_t), 0) < 0) {
        printf("Send2 failed");
        return 1;
    }

    if (recv(sock1, ccount1, 104, 0) < 0) {
        printf("recv1 failed");
    }
    if (recv(sock2, ccount2, 104, 0) < 0) {
        printf("recv2 failed");
    }

    for(int i=0; i<26; i++){
        printf("%c %d\n", 'a'+i, ntohl(ccount1[i])+ntohl(ccount2[i]));
    }

    close(sock1);
    close(sock2);

    return 0;
}
