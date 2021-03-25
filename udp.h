/***********************************************************************
 * Filename: udp.h
 * 
 * Authors: Diogo Martins, n.º 93047
 *          Duarte Oliveira, nº94192
 * 
 * Last modification: 23-03-2021
 * 
 * Comments:
 *            
 *                   Functions used in udp.c file
 *                  
 ***********************************************************************/
#ifndef _UDP_H
#define _UDP_H

/* libraries */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/select.h>
#include "structs.h"

/* functions prototype */
int ServerUDP(int port);
int ClientUDP(char *ip,int port,struct sockaddr_in *addr);
int sendUDP(struct sockaddr_in *addr, int fd, void *sendingMessage, size_t dim);
int ReceiveUDP(struct sockaddr_in *addr, int fd, void *recvMessage, int dim);


#endif