/******************************************************************************
* liso.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "Utility.h"
#include "selectEngine.h"

#define ECHO_PORT 9090
#define BUF_SIZE 4096


int main(int argc, char* argv[])
{
    int sock, client_sock;
    socklen_t cli_size;
    struct sockaddr_in cli_addr;
    char buf[BUF_SIZE];

    printf("Server> ");
    fflush(stdout);
    /* open server socket */
    sock = Open_ListenSocket(ECHO_PORT);

    fd_set readfds, testfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        int fd, nread;

        testfds = readfds;
        fd = select(sock+1, &testfds, 
                    (fd_set*)NULL, (fd_set*)NULL, (struct timeval*)NULL);

        if (fd < 1) {
            Liso_error("Error: select failed");
        }

        /* if the user has entered a command, process it */
        if (FD_ISSET(sock, &testfds))
        {
            cli_size = sizeof(cli_addr);
            if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                     &cli_size)) == -1)
            {
                close(sock);
                fprintf(stderr, "Error accepting connection.\n");
                return EXIT_FAILURE;
            }
            //printf("adding client on fd %d ...\n", client_sock);
            FD_SET(client_sock, &readfds);

            //while((readret = recv(client_sock, buf, BUF_SIZE, 0)) > 1)
            //{
            bzero(buf, BUF_SIZE);
            if ((nread = recv(client_sock, buf, BUF_SIZE, 0)) < 0)
            {
                //printf("nread = %d\n", nread);
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error reading from client socket.\n");
                return EXIT_FAILURE;
            }
            
            if (send(client_sock, buf, nread, 0) != nread)
            {
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error sending to client.\n");
                return EXIT_FAILURE;
            }
        }
        else
        {
            ioctl(fd, FIONREAD, &nread);
            if (nread == 0)
            {
                close(fd);
                FD_CLR(fd, &readfds);
                printf("removing client on fd %d ...\n", fd);
            }
        }     
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
