// RCOM 2026/2027
//
// FTP download application
// Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FTP_PORT 21
#define MAX_SIZE 256

struct URL {
    char user[MAX_SIZE];
    char password[MAX_SIZE];
    char host[MAX_SIZE];
    char path[MAX_SIZE];
    char filename[MAX_SIZE];
};

// ftp://[<user>:<password>@]<host>/<url-path>
// devolve 0 se o url for valido, -1 se nao
int parseUrl(const char *input, struct URL *url) {
    // assim nenhum campo passa do MAX_SIZE
    if (strlen(input) >= MAX_SIZE)
        return -1;

    if (strncmp(input, "ftp://", 6) != 0)
        return -1;

    // antes da primeira / fica [<user>:<password>@]<host>, depois fica o caminho
    char server[MAX_SIZE];
    strcpy(server, input + 6);
    char *slash = strchr(server, '/');
    if (slash == NULL)
        return -1;
    *slash = '\0';
    strcpy(url->path, slash + 1);

    // sem @ e modo anonimo
    char *at = strrchr(server, '@');
    if (at == NULL) {
        strcpy(url->user, "anonymous");
        strcpy(url->password, "anonymous");
        strcpy(url->host, server);
    } else {
        *at = '\0';
        char *colon = strchr(server, ':');
        if (colon == NULL)
            return -1;
        *colon = '\0';
        strcpy(url->user, server);
        strcpy(url->password, colon + 1);
        strcpy(url->host, at + 1);
    }

    // o ficheiro e guardado com o nome que vem depois da ultima /
    char *name = strrchr(url->path, '/');
    strcpy(url->filename, name == NULL ? url->path : name + 1);

    if (strlen(url->host) == 0 || strlen(url->filename) == 0)
        return -1;

    return 0;
}

// do getip.c - descobre o ip do host pelo nome (DNS)
int getIp(const char *host, char *ip) {
    struct hostent *h;

    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        return -1;
    }

    strcpy(ip, inet_ntoa(*((struct in_addr *) h->h_addr)));
    return 0;
}

// do clientTCP.c - abre um socket TCP e liga ao servidor
// devolve o descritor do socket
int connectSocket(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }

    return sockfd;
}

// le uma linha da ligacao de controlo, um byte de cada vez ate ao \n
int readLine(int sockfd, char *line) {
    int i = 0;
    char c;

    do {
        if (read(sockfd, &c, 1) <= 0)
            return -1;
        line[i++] = c;
    } while (c != '\n' && i < MAX_SIZE - 1);

    line[i] = '\0';
    return i;
}

// a ultima linha de uma resposta tem o codigo seguido de espaco, ex: "220 Welcome"
// as outras linhas de uma resposta multi-linha sao do tipo "220-Welcome"
int isLastLine(const char *line) {
    return strlen(line) >= 4 && isdigit(line[0]) && isdigit(line[1]) && isdigit(line[2]) && line[3] == ' ';
}

// le a resposta toda do servidor e devolve o codigo
// em response fica a ultima linha (precisamos dela no PASV)
int readResponse(int sockfd, char *response) {
    do {
        if (readLine(sockfd, response) < 0) {
            printf("Error: connection closed by the server\n");
            return -1;
        }
        printf("%s", response);
    } while (!isLastLine(response));

    return atoi(response);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(-1);
    }

    struct URL url;
    if (parseUrl(argv[1], &url) != 0) {
        printf("Error: invalid URL %s\n", argv[1]);
        exit(-1);
    }

    char ip[MAX_SIZE];
    if (getIp(url.host, ip) != 0) {
        printf("Error: could not resolve host %s\n", url.host);
        exit(-1);
    }

    printf("Host name  : %s\n", url.host);
    printf("IP Address : %s\n", ip);

    // ligacao de controlo na porta 21
    int control = connectSocket(ip, FTP_PORT);
    if (control < 0) {
        printf("Error: could not connect to the server\n");
        exit(-1);
    }

    // 220 - servidor pronto
    char response[MAX_SIZE];
    if (readResponse(control, response) != 220) {
        printf("Error: server not ready\n");
        exit(-1);
    }

    if (close(control) < 0) {
        perror("close()");
        exit(-1);
    }

    return 0;
}
