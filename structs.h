/**************************************************************************************************
 * Filename: topologia.h
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

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h> 

/* DEFINE */
#define BUFFER1 10
#define SIZE 100
#define SIZEMESSAGE 300
#define SIZERESPOSTA 600
#define SIZE_XS 500

typedef struct user
{
    char ipaddr[SIZE]; /* IP da interface de rede da aplicação */

    int tcpPort;  /* TCP port where the application accepts sessions from other downstream peers */
    int udpPort; /* UDP port of the access server */
    char nodeServeraddr[SIZE]; /* IP of node server */
    int nodeServerPort; /* Port of node server */
    int tcpSessions; /* number of TCP sessions */
    int node; /*sets value to 1 if is a node, 0 if not */

    

    int debug,display,help;  /* Algumas das flags responsaveis pela apresentação dos dados para o utilizador */

    int numfilhos; /* numero de filhos interligados à aplicação */
    
    int fdaccess; /* file descriptor indicating the socket that establishes communication with the node server */
    int fd_montante; /* Descritor de ficheiro que indica a socket que estabelece comunicação com o pai /aplicação a montante */
    int *fd_jusante; /* Array com os descritores de ficheiros das sockets dos filhos */
    int fd_recv; /* Descritor de ficheiro da socket da ligacao tcp a fonte */
    char *info_montante, **info_jusante; /* Buffer de armazenamento dos dados que são partilhado entre as aplicações */
    char **PopAddr; /* Armazenamento do endereço ip e do porto tcp dos pontos de acesso */ 
    char **FilhosAddr; /* Armazenamento do endereço ip e do porto tcp dos filhos */
    int imprimirArvore; /* Variavel auxiliar que indica o numero para imprimir a arvore consoante o numero de filhos */
    int *filhoextra; /* Flag para receber informacao extra do tree reply vindo dos filhos */
}user;

