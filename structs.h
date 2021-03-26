/**************************************************************************************************
 * Filename:  structs.h
 * 
 * Authors: Duarte Oliveira, nº 94192
 *          Diogo Martins,   nº 93047
 * 
 * Last modification: 24/03/2021
 * 
 * Description : Structures used to build the topology required for the problem 
 * 
 **************************************************************************************************/
#ifndef _STRUCTS_H
#define _STRUCTS_H

/* LIBRARIES */
#include <stdio.h>
#include <stdlib.h> 

/* DEFINE */
#define SIZE 100

typedef struct user
{
    char ipaddr[SIZE]; /* Interface IP */
    int tcpPort;  /* TCP port where the application accepts sessions from other downstream peers */
    int udpPort; /* UDP port of the access server */
    char nodeServeraddr[SIZE]; /* IP of node server */
    int nodeServerPort; /* Port of node server */
    int tcpSessions; /* number of TCP sessions */
}user;

#endif