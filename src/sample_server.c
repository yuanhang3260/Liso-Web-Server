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
#include "Utility.h"
#include "SelectEngine.h"
#include "Logger.h"
#include "FileIO.h"

#define DFT_PORT 9090


int main(int argc, char **argv)
{
    /** initialize logger */
    initLogger("./log.txt");
    logger(CUR_LOG_LEVEL, "Start Logging ...\n\n");

    /* open server listen socket */
    int listenfd; 
    int port = (argc == 1)? DFT_PORT : atoi(argv[1]);
    listenfd = Open_ListenSocket(port);
    logger(CUR_LOG_LEVEL, 
           "Server socket %d created, listening port %d ...\n", listenfd, port);

    /* init select pool */
    pool_t pool;
    init_pool(listenfd, &pool);
    logger(CUR_LOG_LEVEL, 
           "Select Pool initialized with %d slots ...\n\n", FD_SETSIZE);

    /* begin loop */
    int loop_num = 0;
    while (1)
    {
        printf("loop %d\n", ++loop_num);

        /* Wait for listening/connected descriptor(s) to become ready */
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        /* If listening descriptor ready, add new client to pool */
        check_server(listenfd, &pool);                
        
        /* Serve each ready connected client */
        check_clients(&pool);

        printf("\n");
    }

    /** terminate logging */
    endLog();
}


