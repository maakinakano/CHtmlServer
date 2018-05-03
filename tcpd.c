#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define QUEUELIMIT 5
#define URL_SIZE 2048
#define LINE_SIZE 2048
#define DOCUMENT_ROOT "C:\\Users\\waon\\Desktop\\workspace\\cockrobin\\http\\"

void html(int fd, char *msg);
void send_msg(int fd, const char *msg);
void send_file(int fd, FILE *fp);
FILE* open_file(char *url);
void parse_url(char *url, const char *request);


const char header[] =   "HTTP/1.0 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "\r\n";

int main(int argc, char* argv[]) {

    int servSock; //server socket descripter
    int clitSock; //client socket descripter
    struct sockaddr_in servSockAddr; //server internet socket address
    struct sockaddr_in clitSockAddr; //client internet socket address
    unsigned short servPort; //server port number
    unsigned int clitLen; // client internet socket address length
    char buf[2048];
    char inbuf[2048];
    char url[URL_SIZE];

    if ( argc != 2) {
        fprintf(stderr, "port number 8080.\n");
        servPort = 8080;
    } else if ((servPort = (unsigned short) atoi(argv[1])) == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }

    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("socket() failed.");
        exit(EXIT_FAILURE);
    }

    memset(&servSockAddr, 0, sizeof(servSockAddr));
    servSockAddr.sin_family      = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port        = htons(servPort);

    if (bind(servSock, (struct sockaddr *) &servSockAddr, sizeof(servSockAddr) ) < 0 ) {
        perror("bind() failed.");
        exit(EXIT_FAILURE);
    }

    if (listen(servSock, QUEUELIMIT) < 0) {
        perror("listen() failed.");
        exit(EXIT_FAILURE);
    }

    while(1) {
        clitLen = sizeof(clitSockAddr);
        if ((clitSock = accept(servSock, (struct sockaddr *) &clitSockAddr, &clitLen)) < 0) {
            perror("accept() failed.");
            exit(EXIT_FAILURE);
        }
        memset(inbuf, 0, sizeof(inbuf));
        recv(clitSock, inbuf, sizeof(inbuf), 0);
        // 本来ならばクライアントからの要求内容をパースすべきです
        parse_url(url, inbuf);
        html(clitSock, url);
        close(clitSock);
    }

    return EXIT_SUCCESS;
}

void parse_url(char *url, const char *request) {
    int i;
    char *start = strstr(request, "GET /")+5;
    char *end = strstr(request, " HTTP/");
    char temp[URL_SIZE];
    for(i=0; start+i!=end; i++) {
        temp[i] = start[i];
    }
    temp[i] = '\0';
    //お気に入り画像(favicon.ico)のリクエストは無視
    if(strcmp(temp, "favicon.ico") == 0) {return;}
    if(strcmp(temp+i-5, ".html") != 0 && strcmp(temp+i-4, ".hml") != 0){return;}
    snprintf(url, URL_SIZE, "%s%s", DOCUMENT_ROOT, temp);
}

void html(int fd, char *url) {
    FILE *fp;
    fp = open_file(url);
    if((fp == NULL)) {
        //404
        printf("wakannnyai\n");
    } else {
        //200 OK
        send_msg(fd, header);
        send_file(fd, fp);
    }
}

FILE* open_file(char* url) {
    FILE *fp;
    fp = fopen(url, "r");
    return fp;
}

void send_file(int fd, FILE *fp) {
    char line[LINE_SIZE];
    char temp[LINE_SIZE];
    fgets(line, LINE_SIZE, fp);
    snprintf(temp, LINE_SIZE, "%s\r\n", line);
    send_msg(fd, temp);
    send_msg(fd, "\r\n");
    fclose(fp);
}

void send_msg(int fd, const char *msg) {
    int len = strlen(msg);
    // 指定されたメッセージ`msg`をソケットに送信
   if (write(fd, msg, len) != len) {
       fprintf(stderr, "error: writing.");
   }
}
