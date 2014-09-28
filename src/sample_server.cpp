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
#include <time.h>
#include "Utility.h"
#include "SelectEngine.h"
#include "Logger.h"
#include "FileIO.h"
#include "sample_server.h"

#define DFT_PORT 9090

using namespace std;


int main(int argc, char **argv)
{
    /** initialize logger */
    Logger logger("./log.txt");
    Logger::log("Start Logging *******\n\n");

    /* init select pool - create pool and init listen socket*/
    SelPool pool((argc > 1? atoi(argv[1]) : DFT_PORT));
    Logger::log("Select Pool initialized with %d slots ...\n\n", FD_SETSIZE);

    /* init FileIO */
    FileIO::initFileIO("./lockfile", "./static_site", "./cgi-bin");
    
    /* begin loop */
    int loop_num = 1;
    while (1)
    {
        sleep(1);
        printf("-------------------------- loop %d -----------------------------\n", loop_num++);
        //pool.print_clients();

        /* Wait for listening/connected descriptor(s) to become ready */
        pool.Select();
        
        /* Serve each ready connected client */
        pool.check_clients();

        /* If listening descriptor ready, add new client to pool */
        pool.check_server();

        printf("\n***\n");
    }
    
    /** terminate logging */
    logger.endLog();
}


