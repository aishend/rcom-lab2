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
#define BUF_SIZE 1024

struct URL {
    char user[MAX_SIZE];
    char password[MAX_SIZE];
    char host[MAX_SIZE];
    char path[MAX_SIZE];
    char filename[MAX_SIZE];
};

// ftp://[<user>:<password>@]<host>/<url-path>
int parseUrl(const char *input, struct URL *url) {
    // se for maior que MAX_SIZE nao cabe nos campos
    if (strlen(input) >= MAX_SIZE)
        return -1;

    if (strncmp(input, "ftp://", 6) != 0)
        return -1;

    // tiramos o ftp:// e cortamos na primeira /, o que vem depois e o caminho
    char server[MAX_SIZE];
    strcpy(server, input + 6);
    char *slash = strchr(server, '/');
    if (slash == NULL)
        return -1;
    *slash = '\0';
    strcpy(url->path, slash + 1);

    // se nao tiver @ entramos como anonymous
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

    // o nome do ficheiro e o que esta depois da ultima /
    char *name = strrchr(url->path, '/');
    strcpy(url->filename, name == NULL ? url->path : name + 1);

    if (strlen(url->host) == 0 || strlen(url->filename) == 0)
        return -1;

    return 0;
}

// igual ao getip.c mas guarda o ip em vez de imprimir
int getIp(const char *host, char *ip) {
    struct hostent *h;

    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        return -1;
    }

    strcpy(ip, inet_ntoa(*((struct in_addr *) h->h_addr)));
    return 0;
}

// do clientTCP.c, devolve o socket ja ligado ao servidor
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

// le byte a byte ate ao \n
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

// le a resposta do servidor e devolve o codigo, em response fica a ultima linha
int readResponse(int sockfd, char *response) {
    // as respostas com varias linhas comecam com "220-" e acabam quando aparece "220 "
    do {
        if (readLine(sockfd, response) < 0) {
            printf("Error: connection closed by the server\n");
            return -1;
        }
        printf("%s", response);
    } while (!isdigit(response[0]) || response[3] != ' ');

    return atoi(response);
}

// manda o comando com \r\n no fim e le logo a resposta
int sendCommand(int sockfd, const char *command, const char *arg, char *response) {
    char buf[2 * MAX_SIZE];

    if (arg == NULL)
        sprintf(buf, "%s\r\n", command);
    else
        sprintf(buf, "%s %s\r\n", command, arg);

    printf("%s", buf);
    if (write(sockfd, buf, strlen(buf)) < 0) {
        perror("write()");
        return -1;
    }

    return readResponse(sockfd, response);
}

// 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
// porta = p1 * 256 + p2
int parsePassive(const char *response, char *ip, int *port) {
    int h1, h2, h3, h4, p1, p2;

    const char *start = strchr(response, '(');
    if (start == NULL)
        return -1;
    if (sscanf(start, "(%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
        return -1;

    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    *port = p1 * 256 + p2;
    return 0;
}

// le da ligacao de dados e escreve no ficheiro ate o servidor fechar (read da 0)
long saveFile(int sockfd, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen()");
        return -1;
    }

    char buf[BUF_SIZE];
    long total = 0;
    int bytes;
    while ((bytes = read(sockfd, buf, BUF_SIZE)) > 0) {
        fwrite(buf, 1, bytes, file);
        total += bytes;
    }

    fclose(file);

    if (bytes < 0) {
        perror("read()");
        return -1;
    }
    return total;
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

    // login, se o user nao precisar de password o servidor responde logo 230
    int code = sendCommand(control, "USER", url.user, response);
    if (code == 331)
        code = sendCommand(control, "PASS", url.password, response);
    if (code != 230) {
        printf("Error: login failed\n");
        exit(-1);
    }

    // TYPE I e o modo binario
    if (sendCommand(control, "TYPE", "I", response) != 200) {
        printf("Error: could not set binary mode\n");
        exit(-1);
    }

    // no modo passivo somos nos que ligamos ao servidor, ele diz o ip e a porta
    if (sendCommand(control, "PASV", NULL, response) != 227) {
        printf("Error: could not enter passive mode\n");
        exit(-1);
    }

    char dataIp[MAX_SIZE];
    int dataPort;
    if (parsePassive(response, dataIp, &dataPort) != 0) {
        printf("Error: invalid passive mode response\n");
        exit(-1);
    }
    printf("Data connection: %s:%d\n", dataIp, dataPort);

    // ligacao de dados
    int data = connectSocket(dataIp, dataPort);
    if (data < 0) {
        printf("Error: could not open the data connection\n");
        exit(-1);
    }

    // o rebex responde 125 em vez de 150
    code = sendCommand(control, "RETR", url.path, response);
    if (code != 150 && code != 125) {
        printf("Error: could not retrieve %s\n", url.path);
        exit(-1);
    }

    long bytes = saveFile(data, url.filename);
    if (bytes < 0) {
        printf("Error: could not save %s\n", url.filename);
        exit(-1);
    }
    close(data);

    // 226 - transferencia completa
    if (readResponse(control, response) != 226) {
        printf("Error: transfer not complete\n");
        exit(-1);
    }

    sendCommand(control, "QUIT", NULL, response);

    if (close(control) < 0) {
        perror("close()");
        exit(-1);
    }

    printf("File saved: %s (%ld bytes)\n", url.filename, bytes);
    return 0;
}
