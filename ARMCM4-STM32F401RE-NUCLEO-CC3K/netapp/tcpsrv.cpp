#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "tcpsrv.h"
#include "cc3000_chibios_api.h"
#include "socket.h"
#include "utility.h"
#include "netapp.h"
#include <string.h>

#define INADDR_ANY 0x00000000

sockaddr_in serveraddr;
sockaddr_in clientaddr; /* client addr */

int srvsocket, clientsocket;
struct hostent * hostp;
char buf[100]; // , *hostaddrp;
int clientlen, n;

bool_t tcpsrv_init(void)
{
    ccprint("Waiting for connection ... ");
    while (cc3000AsyncData.connected != 1)
    {
        chThdSleep(MS2ST(5));
    }
    ccprint("Connected!\r\n");
    ccprint("Waiting for DHCP...");
    while (cc3000AsyncData.dhcp.present != 1)
    {
        chThdSleep(MS2ST(5));
    }
    ccprint("Received!\r\n");

    return TRUE;
}

void tcpsrv_start(void)
{
    int optval;

    ccprint("Creating server socket ...\r\n");
    while(1)
    {
        srvsocket = socket(AF_INET, SOCK_STREAM, 0);
        if (srvsocket < 0)
        {
            ccprint("Error opening socket ...\r\n");
            chThdSleep(MS2ST(1000));

            continue;
        }
        //optval = 1;
        //setsockopt(srvsocket, SOL_SOCKET, SO_REUSEADDR, 
        //    (const void *) &optval, sizeof(int));
        optval= SOCK_OFF;

        if (setsockopt(srvsocket, 
                    SOL_SOCKET,
                    SOCKOPT_ACCEPT_NONBLOCK,
                    &optval,
                    sizeof(optval)) == -1)
        {
            ccprint("Failed to set socket accept option to blocking mode ...\r\n");
            closesocket(srvsocket);
            chThdSleep(MS2ST(1000));
            continue;
        }

        /*
         * build the server's Internet address
         */
        memset((char *) &serveraddr, 0, sizeof(serveraddr));

        /* this is an Internet address */
        serveraddr.sin_family = AF_INET;

        /* let the system figure out our IP address */
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

        /* this is the port we will listen on */
        serveraddr.sin_port = htons(TCPSRV_PORT);

        /* 
         * bind: associate the parent socket with a port 
         */
        if (bind(srvsocket, (sockaddr *) &serveraddr, 
                    sizeof(serveraddr)) < 0) 
        {
            ccprint("ERROR on binding\r\n");
            closesocket(srvsocket);
            chThdSleep(MS2ST(1000));
            continue;
        }

        /* 
         * listen: make this socket ready to accept connection requests 
         */
        if (listen(srvsocket, 5) < 0) /* allow 5 requests to queue up */ 
        {
            ccprint("ERROR on listen\r\n");
            closesocket(srvsocket);
            chThdSleep(MS2ST(1000));
            continue;
        }


        clientlen = sizeof(clientaddr);
        while(1)
        {
            /* 
             * accept: wait for a connection request 
             */
            clientsocket = accept(srvsocket, (sockaddr *) &clientaddr, (socklen_t *) &clientlen);
            if (clientsocket < 0) 
            {
                ccprint("ERROR on accept\r\n");
                closesocket(srvsocket);
                chThdSleep(MS2ST(2000));
                continue;
            }


            //hostaddrp = inet_ntoa(clientaddr.sin_addr);
            //if (hostaddrp == NULL)
            //{
            //    fprintf(stderr,"ERROR on inet_ntoa\n");
            //    return FALSE;
            //}

            ccprint("Server established connection\r\n"); 

            memset(buf, 0,  100);
            n = recv(clientsocket, buf, 100, 0);
            if (n < 0) 
            {
                ccprint("ERROR reading from socket\r\n");
                closesocket(clientsocket);
                chThdSleep(MS2ST(1000));
                continue;
            }

            ccprint("%s\r\n", buf, n +1);

            closesocket(clientsocket);
        }
    }
}

