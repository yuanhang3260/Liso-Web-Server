/* 
 * @file sample_server.c - A concurrent echo server based on select.
 *
 * @author Hang Yuan (Andrew ID: hangyuan)
 * @bug no bugs found yet
 */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include "Utility.h"
#include "SelectEngine.h"
#include "Logger.h"
#include "FileIO.h"

#define DFT_PORT 9090

using namespace std;


int main(int argc, char **argv)
{
    /** initialize logger */
    Logger logger("./log.txt");
    Logger::log("Start Logging ...\n\n");

    /* init select pool - create pool and init listen socket*/
    SelPool pool((argc > 1? atoi(argv[1]) : DFT_PORT));
    Logger::log("Select Pool initialized with %d slots ...\n\n", FD_SETSIZE);
    
    /* begin loop */
    int loop_num = 1;
    while (1)
    {
        printf("loop %d, selecting ...\n", loop_num++);
        
        /* Wait for listening/connected descriptor(s) to become ready */
        pool.Select();
        
        /* If listening descriptor ready, add new client to pool */
        pool.check_server();                
        
        /* Serve each ready connected client */
        pool.check_clients();

        printf("\n");
    }

    /** terminate logging */
    logger.endLog();
}


