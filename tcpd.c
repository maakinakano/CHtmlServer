#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define QUEUELIMIT 5
#define URL_SIZE 256
#define LINE_SIZE 2048
#define DOCUMENT_ROOT "C:\\Users\\waon\\Desktop\\workspace\\cockrobin\\http\\root\\"

typedef struct extension {
    char* name;
    char* headerType;
} Extension;

void html(int fd, char *msg);
void send_msg(int fd, const char *msg);
void send_file(int fd, FILE *fp);
FILE* open_file(char *url);
void parse_url(char *url, const char *request);

const Extension support_exte[] = {
    {"others", "text/plain"},
    {".html", "text/html"},
    {".hml", "text/html"},
    {".css", "text/css"},
    {".jpg", "image/jpeg"},
    {".png", "image/png"},
};

const char header_template[] =  "HTTP/1.0 200 OK\r\n"
                                "Content-Type: %s\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "\r\n";

const char header404[] = "HTTP/1.0 404 Not Found\r\n"
                         "Status: 404\r\n"
                         "Content-Type: text/html\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "<h1>404 Nothing There!</h1>";

int main(int argc, char* argv[]) {

    int servSock; //server socket descripter
    int clitSock; //client socket descripter
    struct sockaddr_in servSockAddr; //server internet socket address
    struct sockaddr_in clitSockAddr; //client internet socket address
    unsigned short servPort; //server port number
    unsigned int clitLen; // client internet socket address length
    char buf[2048];
    char request[2048];
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
        memset(request, 0, sizeof(request));
        memset(url, 0, sizeof(url));
        recv(clitSock, request, sizeof(request), 0);
        parse_url(url, request);
        html(clitSock, url);
        close(clitSock);
    }

    return EXIT_SUCCESS;
}

void parse_url(char *url, const char *request) {
    int i;
    char *start = strstr(request, "GET /");
    char *end = strstr(request, " HTTP/");
    if(start == NULL || end == NULL) {
        return;
    }
    start+=5;
    for(i=0; start+i!=end; i++) {
        url[i] = start[i];
    }
    url[i] = '\0';
}

int detect_extension(const char *url) {
    int i, len = strlen(url);
    int num = sizeof(support_exte)/sizeof(Extension);
    for(i=1; i<num; i++) {
        const Extension* exte = support_exte+i;
        int exte_len = strlen(exte->name);
        if(len <= exte_len){continue;}
        if(strcmp(url+len-exte_len, exte->name) == 0){
            return i;
        }
    }
    //supportされていないなら0を返す
    return 0;
}

void html(int fd, char *url) {
    FILE *fp = NULL;
    int exte_num = detect_extension(url);
    char header[LINE_SIZE];
    char request_file[URL_SIZE];
    snprintf(header, LINE_SIZE, header_template, support_exte[exte_num].headerType);
    snprintf(request_file, URL_SIZE, "%s%s", DOCUMENT_ROOT, url);
    printf("%s\n", request_file);
    fp = open_file(request_file);
    if(fp == NULL) {
        //404
        send_msg(fd, header404);
    } else {
        //200 OK
        send_msg(fd, header);
        send_file(fd, fp);
    }
}

FILE* open_file(char* url) {
    FILE *fp=NULL;
    fp = fopen(url, "rb");
    return fp;
}

void send_file(int fd, FILE *fp) {
    unsigned char line[LINE_SIZE];
    int size;
    while(size = fread(line, sizeof(unsigned char), LINE_SIZE, fp)) {
        write(fd, line, size);
    }
    fclose(fp);
}

void send_msg(int fd, const char *msg) {
    int len = strlen(msg);
    // 指定されたメッセージ`msg`をソケットに送信
   if (write(fd, msg, len) != len) {
       fprintf(stderr, "error: writing.");
   }
}
