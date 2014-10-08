#include "ch.h"
#include "hal.h"
#include "tcpsend.h"
#include "hwdefs.h"
#include "chprintf.h"
#include "cc3000_chibios_api.h"
#include "socket.h"
#include "utility.h"
#include "netapp.h"
#include "dht11.h"
#include <string.h>

#define OUTBUF_SIZ          100

int sock;
sockaddr_in destAddr;
sockaddr fromAddr;
char txBuffer[OUTBUF_SIZ];
uint8_t sensorData[DHT11_DATA_SIZ];

bool_t tcpsnd_init(void)
{
    // Wait untill connected
    ccprint("Connecting to AP ... ");
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
    show_cc3_dhcp_info((BaseSequentialStream *) &SERIAL_DRIVER);

    return TRUE;
}

void tcpsnd_start(void)
{
    uint32_t remoteHostIp;
    bool_t connected;
    uint16_t sz;

    while (1) 
    {
       connected = FALSE;

       if (gethostbyname(REMOTE_IP, strlen(REMOTE_IP), &remoteHostIp) < 0)
       {
           ccprint("Failed resolving address [%s] ...\r\n",
                REMOTE_IP);
           chThdSleepMilliseconds(1000);
           continue;
       }

       destAddr.sin_family = AF_INET;
       destAddr.sin_port = htons(REMOTE_PORT);
       destAddr.sin_addr.s_addr = htonl(remoteHostIp);


       ccprint("Creating socket...");
       if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP)) == -1)
       {
           ccprint("socket() returned error.\r\n");
           chThdSleepMilliseconds(1000);
           continue;
       }
       ccprint("Created!\r\n");

       ccprint("Connecting to server ...");
       if ( connect(sock, (sockaddr *) &destAddr, sizeof(destAddr)) == -1 )
       {
           ccprint("Error connecting to server ...\r\n");
           chThdSleepMilliseconds(1000);

           closesocket(sock);
           continue;
       }

       ccprint("connected!\r\n");
       connected = TRUE;

       while(connected == TRUE)
       {
           // Get the data from the sensor
           if (read_DHT11(sensorData) != DHT11_OK)
           {
               ccprint("Failed reading sensor data ...\r\n");
               chThdSleepMilliseconds(1000);
               continue;
           }
           // Format the message
           memset(txBuffer, 0, OUTBUF_SIZ);
           sz = chsnprintf(txBuffer, OUTBUF_SIZ, "TEMP[%f]HUMIDTY[%f]\n", dht11_gettemp(sensorData),
                    dht11_gethumidity(sensorData));

           // Send data to server
           if (send(sock, (const void *) txBuffer, sz, 0) == -1)
           {
               connected = FALSE;
               // close the socket
               closesocket(sock);
           }
       }
    }
}


