/***********************************************************************
 * Filename: tcp.c
 * 
 * Authors: Diogo Martins,   nº93047
 *          Duarte Oliveira, nº94192
 * 
 * Last modification: 23-03-2021
 * 
 *  Comments:
 *            
 *           Auxiliary functions for establishing a connection
 *                        using the TCP protocol
 ***********************************************************************
 *                  TCP – Transmission Control Protocol           
 ***********************************************************************/

/* LIBRARIES */
#include "tcp.h"


int ClientTCP(char *ip, int port)
{
    int fd = -1; /* socket descriptor */
    int n=0;
    char inputport[10];
    struct sockaddr_in server; /* estrutura para armazenar os dados */

    sprintf(inputport,"%d",port); /* Conversao de int em char */
    inputport[9]='\0';

    server.sin_family = AF_INET; 
    server.sin_port = htons(port);

    n = inet_aton(ip,&(server.sin_addr));
    if ( n == 0)
    {
        fprintf(stderr,"[TCP] Error: IP Address (%s) is not valid\n",ip);
        return -1;
    }
    
    fd = socket (AF_INET,SOCK_STREAM,0);
    
    if ( fd == -1 )
    {
        fprintf(stderr,"[TCP] Error: The socket was not created\n");
        return -1;
    }

    n = connect(fd,(struct sockaddr *)&server, sizeof(struct sockaddr_in));

    if ( n == -1 )
    {
        fprintf(stderr,"[TCP] Error: The connection to the server was unsuccessful\n");
        return -1;
    }
    return fd;
}


int ServerTCP(int port)
{
    int fd = -1 , n=0;
    char inputport[10];
    struct addrinfo hints, *res;


    memset(inputport,0,10);
    memset(&hints, 0, sizeof(hints));


    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICSERV;

    sprintf(inputport,"%d", port); 

    inputport[9]='\0'; 

    n = getaddrinfo(NULL, inputport, &hints, &res);
    if (n != 0)
    {
        fprintf(stderr,"[TCP] Error: getaddrinfo : %s\n",gai_strerror(n));
        freeaddrinfo(res);
        return -1;
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1)
    {
        fprintf(stderr,"[TCP] Error: The socket was not created\n");
        freeaddrinfo(res);
        return -1;
    }

    n = bind(fd,res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        fprintf(stderr,"[TCP] Error: BIND\n");
        freeaddrinfo(res);
        return -1;
    }

    n = listen(fd, 5);
    if (n == -1)
    {
        fprintf(stderr,"[TCP] Error: LISTEN\n");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);

    return fd;
}

int SendTCP(int fd, char* protocol, int dim)
{
    int bytesSent = 0;
    char buffer[3] = {0};

    bytesSent = write(fd,protocol, dim);
    if (bytesSent < 0)
    {
        fprintf(stderr,"[TCP] Error: Message not sent\n");
        return -1;
    }
   
    sscanf(protocol,"%2s ", buffer);
    fprintf(stderr,"Mensagem sent by TCP of %d bytes | %s\n", bytesSent,buffer);
    

    return 0;
}


int ReceiveTCP(int fd, char *response, int dim)
{
    int recvBytes = 0;
    char buffer[3], *closer = NULL;
    
    memset(response,0,dim);
    memset(buffer,0,3);
    recvBytes = read(fd, response, dim-1);
    if(recvBytes < 0)
    {
        fprintf(stderr,"[TCP] Error: Message not received\n");
        return -1;
    }

    if (recvBytes == dim-1)
    {
    	closer = buffer;
    	closer[dim-1]='\0';
    }

    sscanf(response,"%2s ", buffer);
    fprintf(stderr,"Mensagem received by TCP of %d bytes | %s\n", recvBytes,buffer);
    

    return recvBytes;
}

int AcceptTCP(int fd, struct sockaddr_in *addr)
{
    int fd_cliente = -1;
    socklen_t addr_size = sizeof *addr , *storeAddress = NULL;

    if (addr != NULL)
    {
        addr_size = sizeof(struct sockaddr_in);
        storeAddress = &addr_size;
    }

    fd_cliente = accept(fd, (struct sockaddr*) addr, storeAddress);
    if (fd_cliente == -1)
    {
        fprintf(stderr,"[TCP] Error: Accepting message\n");
        return -1;
    }

    if (addr_size != sizeof(struct sockaddr_in) && (storeAddress != NULL))
        fprintf(stderr,"[TCP] Error: Client address is invalid\n");

    return fd_cliente;
}