/***********************************************************************
 * Filename: tcp.h
 * 
 * Authors: Diogo Martins, n.º 93047
 *          Duarte Oliveira, nº94192
 * 
 * Last modification: 23-03-2021
 * 
 * Comments:
 *            
 *                   Functions used in tcp.c file
 *                  
 ***********************************************************************/
#ifndef _TCP_H
#define _TCP_H

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include "structs.h"


/* Protótipo de Funcoes */
int ClientTCP(char *ip, int port);
int ServerTCP(int port);
int SendTCP(int fd, char* protocol, int dim);
int ReceiveTCP(int fd, char *response, int dim);
int AcceptTCP(int fd, struct sockaddr_in *addr);


#endif