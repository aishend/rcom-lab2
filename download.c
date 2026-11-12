// RCOM 2026/2027
//
// FTP download application
// Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    printf("User     : %s\n", url.user);
    printf("Password : %s\n", url.password);
    printf("Host     : %s\n", url.host);
    printf("Path     : %s\n", url.path);
    printf("File     : %s\n", url.filename);

    return 0;
}
