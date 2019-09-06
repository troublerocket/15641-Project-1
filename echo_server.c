/******************************************************************************
* echo_server.c                                                               *
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
#include <unistd.h>
#include "parse.h"

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, client_sock, maxfdp;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    struct fd_set fds, read_fds;
    struct timeval timeout={0,0}; 

    FD_ZERO(&fds);
    FD_ZERO(&read_fds);

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    maxfdp = sock;
    FD_SET(sock, &fds);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
       read_fds = fds; 
       if(select(maxfdp + 1, &read_fds, NULL, NULL, &timeout) == -1)
       {
           exit(4);
       } 
       for(int i = 0; i <= maxfdp; i++){
           if(FD_ISSET(i, &read_fds)){
               if(sock == i){
                   cli_size = sizeof(cli_addr);
                   client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                 &cli_size);
                    if(client_sock == -1){
                        fprintf(stderr, "Error accepting connection.\n");
                        //return EXIT_FAILURE;
                    }
                    else{
                        FD_SET(client_sock, &fds);
                        maxfdp = client_sock > maxfdp ? client_sock : maxfdp;
                    }
                }else{
                    memset(buf, 0, BUF_SIZE);
                    readret = recv(i, buf, BUF_SIZE, 0);
                    if (send(i, buf, readret, 0) != readret)
                    {
                        close_socket(i);
                        close_socket(sock);
                        fprintf(stderr, "Error sending to client.\n");
                        //return EXIT_FAILURE;
                    }
                    if (readret >= 1){
                        Request* r = parse(buf, readret, i);
                        if(r == NULL){
                            send(i, "HTTP/1.1 400 Bad Request\r\n", 
                                strlen("HTTP/1.1 400 Bad Request\r\n"), 0);
                        }else{
                            send(i, buf, readret, 0);
                        }
                    }else{
                        close_socket(i);
                        FD_CLR(i, &fds);
                    }



                }
            }
        }   
       //cli_size = sizeof(cli_addr);
       /* 
       if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                 &cli_size)) == -1)
       {
           close(sock);
           fprintf(stderr, "Error accepting connection.\n");
           return EXIT_FAILURE;
       }
       */
       
       //readret = 0;
       /*
       while((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
       {
           if (send(client_sock, buf, readret, 0) != readret)
           {
               close_socket(client_sock);
               close_socket(sock);
               fprintf(stderr, "Error sending to client.\n");
               return EXIT_FAILURE;
           }
           memset(buf, 0, BUF_SIZE);
       } 

       if (readret == -1)
       {
           close_socket(client_sock);
           close_socket(sock);
           fprintf(stderr, "Error reading from client socket.\n");
           return EXIT_FAILURE;
       }

       if (close_socket(client_sock))
       {
           close_socket(sock);
           fprintf(stderr, "Error closing client socket.\n");
           return EXIT_FAILURE;
       }
       */
    }

    close_socket(sock);

    return EXIT_SUCCESS;
    
}
