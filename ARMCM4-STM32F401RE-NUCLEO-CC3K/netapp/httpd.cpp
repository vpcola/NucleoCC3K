#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "httpd.h"
#include "cc3000_chibios_api.h"
#include "socket.h"
#include "utility.h"
#include "netapp.h"
#include "hwdefs.h"
#include <string.h>


#define HTTPD_RX_BUFFER_SIZE  CC3000_RX_BUFFER_SIZE + 100
#define HTTPD_TX_BUFFER_SIZE  CC3000_TX_BUFFER_SIZE 

#define INADDR_ANY  0x00000000 // 0.0.0.0

int clients[MAX_CONNECTIONS];
#if USE_THREADED_HANDLER 
static Thread * pConnectionMonitorThd = NULL;
#endif

INT16 ulServerSocket, ulClientSocket;
static bool_t ulServerSocketOpen=FALSE;

sockaddr tClientSocketAddr;
socklen_t tClientSocketAddrLen;

static char rx_buffer[HTTPD_RX_BUFFER_SIZE];
// static char tx_buffer[HTTPD_TX_BUFFER_SIZE];

const char * http_strings[] = 
{
    "HTTP/1.0 400 Bad request\r\n",
    "HTTP/1.0 404 File not found\r\n",
};

#define HTTP_400 0
#define HTTP_404 1

void http_404(INT16 clientsocket)
{
    send(clientsocket, 
        http_strings[HTTP_400],
        strlen(http_strings[HTTP_400]),
        0);

    closesocket(clientsocket);        
}

void handle_client(INT16 clientsocket)
{
    INT16 bytesReceived = 0;
    char *rlines[3]; 

    ccprint("Reading data from client socket ...\r\n");

    if((bytesReceived = recv(clientsocket, 
        rx_buffer, 
        HTTPD_RX_BUFFER_SIZE, 0)) == -1)
    {
        // Close the socket and return
        ccprint("Error receiving from client socket ...\r\n");
        closesocket(clientsocket);
        return;
    }
    ccprint("Received %d data from client ..\r\n", bytesReceived);


    /* Look for the request in the first line*/
    rlines[0] = strtok(rx_buffer, " \t\n");
    if (strncmp(rlines[0], "GET ", 4) == 0)
    {
        // GET request, check HTTP version
        rlines[1] = strtok(NULL, " \t");
        rlines[2] = strtok(NULL, " \t\n");

        if ((strncmp(rlines[2], "HTTP/1.0", 8) != 0)
            && (strncmp(rlines[2], "HTTP/1.1", 8) != 0))
        {
            http_404(clientsocket);
        }else
        {
            // Ger the URI
            http_404(clientsocket);
        }
    }
    else
    {
        ccprint("Only able to do GET requests for now ...\r\n");

        closesocket(ulClientSocket);
        return;
    }
}

#if USE_THREADED_HANDLER 
static WORKING_AREA(waHttpdConnectionMonitor, 2048);
static msg_t httpdConnectionMonitor(void *arg)
{
    (void)arg;
    int ulSocketHandleTest;
    if (ulServerSocketOpen == TRUE)
    {
        ccprint("Started accepting connection ...\r\n");
        while(1) 
        {
            ulSocketHandleTest = accept(ulServerSocket, 
                &tClientSocketAddr,
                &tClientSocketAddrLen);

            if (ulSocketHandleTest < 0)
            {
                // Error or timeout accepting client
                // error accepting socket ...
                continue;
            }

            ccprint("Client %d accepted ...\r\n", ulSocketHandleTest);
            // We now have a valie client socket
            ulClientSocket = ulSocketHandleTest;

            handle_client();
        }
    }

    return -1; 
}
#endif

bool_t httpd_init(void)
{
    sockaddr_in tSocketAddr;

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
   
    show_cc3_dhcp_info((BaseSequentialStream *)&SERIAL_DRIVER);
    

    ccprint("Initializing HTTP server ...\r\n");
    if (ulServerSocketOpen == FALSE)
    {
        ccprint("Creating server socket ...\r\n");
        tSocketAddr.sin_family = AF_INET;
        tSocketAddr.sin_port = htons(HTTPD_PORT);
        tSocketAddr.sin_addr.s_addr = htons(INADDR_ANY);
        // Create the socket
        if((ulServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        {
            ccprint("Error creating socket ...\r\n");
            return FALSE;
        }

        ccprint("Binding socket ...\r\n", ulServerSocket);
        // Bind the socket
        if (bind(ulServerSocket, (sockaddr *) &tSocketAddr, sizeof(tSocketAddr)) == -1)
        {
            ccprint("Error binding to the socket ...\r\n");
            return FALSE;
        }

        ccprint("Start listening to the socket ...\r\n");
        if (listen(ulServerSocket, 5) == -1)
        {
            ccprint("Error listening to the socket ...\r\n");
            return FALSE;
        }

        ccprint("Server socket created!\r\n", ulServerSocket);

        ulServerSocketOpen = TRUE;
        return ulServerSocketOpen; 
    }

    ccprint("Returning ...\r\n");
    return FALSE;
}

/* Creates a static thread to start
 monitoring connections on the socket */
void httpd_start(void)
{
    INT32 ulSocketHandleTest, option;
    INT16 bytesReceived = 0;
    char *rlines[3]; 

    ccprint("Starting httpd ...\r\n");
#if USE_THREADED_HANDLER 
    if (pConnectionMonitorThd == NULL)
    {
        ccprint("Creating connection listener ...\r\n");
        pConnectionMonitorThd = chThdCreateStatic(waHttpdConnectionMonitor,
            sizeof(waHttpdConnectionMonitor),
            NORMALPRIO,
            httpdConnectionMonitor,
            NULL);
    }
    else if (chThdTerminated(pConnectionMonitorThd)) 
    {
        chThdRelease(pConnectionMonitorThd);    
        pConnectionMonitorThd = NULL; 
    }
#else
    if (ulServerSocketOpen == TRUE)
    {
        ccprint("Setting blocking mode ...\r\n");
        // Do a blocking accept
        option = SOCK_OFF;

        if (setsockopt(ulServerSocket, 
            SOL_SOCKET,
            SOCKOPT_ACCEPT_NONBLOCK,
            &option,
            sizeof(option)) == -1)
        {
            ccprint("Failed to set socket accept option to blocking mode ...\r\n");
            return;
        }

        while(1) 
        {
            ccprint("Listening for connections ...\r\n");
            if ((ulSocketHandleTest = accept(ulServerSocket, 
                    &tClientSocketAddr,
                    &tClientSocketAddrLen)) == -1)

            {
                // Error or timeout accepting client
                // error accepting socket ...
                ccprint("Error accepting socket ...\r\n");
                continue;
            }

            ccprint("Client accepted ...\r\n");
            // We now have a valie client socket
            ulClientSocket = ulSocketHandleTest;

            if((bytesReceived = recv(ulSocketHandleTest, 
                            rx_buffer, 
                            HTTPD_RX_BUFFER_SIZE, 0)) == -1)
            {
                // Close the socket and return
                ccprint("Error receiving from client socket ...\r\n");
                closesocket(ulClientSocket);
                continue;
            }

            ccprint("Received %d data from client ..\r\n", bytesReceived);

            /* Look for the request in the first line*/
            rlines[0] = strtok(rx_buffer, " \t\n");
            if (strncmp(rlines[0], "GET ", 4) == 0)
            {
                // GET request, check HTTP version
                rlines[1] = strtok(NULL, " \t");
                rlines[2] = strtok(NULL, " \t\n");

                if ((strncmp(rlines[2], "HTTP/1.0", 8) != 0)
                        && (strncmp(rlines[2], "HTTP/1.1", 8) != 0))
                {
                    http_404(ulSocketHandleTest);
                }else
                {
                    // Ger the URI
                    http_404(ulSocketHandleTest);
                }
            }
            else
            {
                ccprint("Only able to do GET requests for now ...\r\n");
                closesocket(ulSocketHandleTest);
            }
        }
    }
#endif
}

