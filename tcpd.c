#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define QUEUELIMIT 5
void html(int fd, char *msg);
void send_msg(int fd, char *msg);

const char header[] =   "HTTP/1.0 200 OK\r\n"
                        "Content-Length: %ld\r\n"
                        "Content-Type: text/html\r\n"
                        "\r\n";

int main(int argc, char* argv[]) {

    int servSock; //server socket descripter
    int clitSock; //client socket descripter
    struct sockaddr_in servSockAddr; //server internet socket address
    struct sockaddr_in clitSockAddr; //client internet socket address
    unsigned short servPort; //server port number
    unsigned int clitLen; // client internet socket address length
    char buf[2048];

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
        html(clitSock, "HELLO\r\n");
        close(clitSock);
    }

    return EXIT_SUCCESS;
}


void html(int fd, char *msg) {
    //headerの作成
    char temp[2000];
    sprintf(temp, header, strlen(msg));
    send_msg(fd, temp);
    send_msg(fd, msg);
}

void send_msg(int fd, char *msg) {
    int len = strlen(msg);
    // 指定されたメッセージ`msg`をソケットに送信
   if (write(fd, msg, len) != len) {
       fprintf(stderr, "error: writing.");
   }
}
