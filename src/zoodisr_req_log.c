#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "zoodisr.h"

int main(int argc, char *argv[])
{
    log_init(LOG_ERR);
    int s, len;
    struct sockaddr_un remote;
    char str[1025];

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, argv[1]);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        log_err("Not able to connect to log UDS %s", argv[1]);
        exit(1);
    }

    log_msg("Connected.");

    while((len = recv(s, str, 1024, 0)) > 0)
    {
        printf("%.*s", len, str);
    }
    close(s);

    return 0;
}
