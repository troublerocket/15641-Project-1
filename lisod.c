/******************************************************************************
* lisod.c                                                                     *
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
#include <sys/select.h>
#include <sys/time.h>
#include "parse.h"

#define ECHO_PORT 4836
#define BUF_SIZE 8192

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
    char *buf;
    fd_set fds, read_fds;
    //struct timeval timeout={0,0}; 
    const char* BAD_REQUEST_RESPONSE = "HTTP/1.1 400 Bad Request\r\n\r\n";
    int BAD_RESPONSE = strlen(BAD_REQUEST_RESPONSE);


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

    FD_SET(sock, &fds);
    maxfdp = sock;
    buf = (char *)malloc(BUF_SIZE);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
       read_fds = fds; 
       if(select(maxfdp + 1, &read_fds, NULL, NULL, NULL) == -1)
       {
            perror("select");
            exit(4);
       } 
       int i = 0;
       for(i = 0; i <= maxfdp; i++){
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
                        if(client_sock > maxfdp){
                            maxfdp = client_sock;
                        }
                    }
                }else{
                    memset(buf, 0, BUF_SIZE);
                    readret = recv(i, buf, BUF_SIZE, 0);
                    /* 
                    int total_len = 0;
                    char * tmp = NULL;
                    while((readret = recv(i, buf, BUF_SIZE, 0)) > 0){
                        total_len += readret;
                        tmp =(char *)realloc(tmp,readret);
                        strcat(tmp,buf);
                        memset(buf, 0, BUF_SIZE);
                    }
                    readret = total_len;
                    if(total_len > BUF_SIZE){
                        buf = (char *)malloc(total_len+1);
                        memset(buf, 0, total_len);
                        strcpy(buf, tmp);
                    }
                    */
                    /* 
                    if (send(i, buf, readret, 0) != readret)
                    {
                        close_socket(i);
                        close_socket(sock);
                        fprintf(stderr, "Error sending to client.\n");
                        //return EXIT_FAILURE;
                    }
                    */
                    if (readret > 0){
                        //printf("received: %s\n",buf);
                        Request* r = parse(buf, readret, i);
                        if(r != NULL){
                            if(strcmp(r->http_method,"GET") == 0 || strcmp(r->http_method,"POST") == 0 || strcmp(r->http_method,"POST") == 0){
                                send(i, buf, readret, 0);
                                printf("send:\n%s",buf);
                            }
                            else{
                            send(i, BAD_REQUEST_RESPONSE, BAD_RESPONSE, 0); 
                            printf("Wrong method:\n");                           
                            }

                        }else{
                            memset(buf, 0, BUF_SIZE);
                            strcpy(buf,BAD_REQUEST_RESPONSE);
                            send(i, buf, strlen(buf), 0);
                            //send(i,buf,readret,0); 
                            printf("Bad Request:%s\n",buf);    
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