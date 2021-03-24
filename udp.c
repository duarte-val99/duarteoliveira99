/***********************************************************************
 * Name of the file : udp.c
 * 
 * Authors: Diogo Martins, nº93...
 *          Duarte Oliveira, nº94192
 * 
 * Last modification: 23-03-2021
 * 
 *  Comments:
 *            
 *           Auxiliary functions for establishing a connection
 *                        using the UDP protocol
 ***********************************************************************
 *                       UDP – User Datagram Protocol                  
 ***********************************************************************/

/*libraries*/
#include "udp.h"

/*****************************************************************************************************
 * ServerUDP(int port)
 * 
 * Input parameters: port -> integer that stores the port to link to UDP server
 *                                  
 * Output parameters: fd -> integer that returns the descriptor file of the server 
 * 
 * Descrição:   Criação de um servidor UDP com o porto especificado à entrada, retornando um file
 *              descriptor.
 ****************************************************************************************************/
int ServerUDP(int port)
{
    int fd = -1, n = 0;
    char inputport[10];
    struct addrinfo hints, *res;

    /* sets port string to 0 */
    memset(inputport,0,10); 

    memset(&hints, 0, sizeof(hints)); /* sets addrinfo structure to 0 */
    hints.ai_family = AF_INET; /* protocol is IPv4 */
    hints.ai_socktype = SOCK_DGRAM; /* communication is using UDP */
    hints.ai_flags = AI_PASSIVE|AI_NUMERICSERV;

	/* converts input port to the string port */
    sprintf(inputport,"%d", port);
    inputport[9]= '\0';

    /* allocates and initializes a list of addrinfo structures for each node */
    n = getaddrinfo(NULL, port, &hints, &res);
    if(n != 0)
    {
        fprintf(stderr,"[UDP SERVER] ERROR: Socket address not obtained\n");
        freeaddrinfo(res);
        return -1;
    }
    
    /* UDP socket */
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fd == -1)
    {
        fprintf(stderr,"[UDP SERVER] Error: Descriptor file not created in the socket\n");
        freeaddrinfo(res);      
        return -1;
    }

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        fprintf(stderr,"[UDP SERVER] Error: Server info not registered\n");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return fd;
}

/*****************************************************************************************************
 * ClientUDP(char *ip, int port, struct sockaddr_in *addr)
 * 
 * Parâmetros de Entrada:   ip -> IP para o qual o cliente UDP se vai ligar
 *                          port -> porto para o qual o cliente UDP se vai ligar
 *                          addr -> estrutura para manipular endereços de internet
 * 
 * Parâmetros de Saída:     int -> retorna 0 se não houver erros, caso contrário retorna -1
 * 
 * Descrição:   Estabelece uma ligação UDP com um servidor, especificado pelos parâmetros à entrada
 * 
 ****************************************************************************************************/
int ClientUDP(char *ip, int port, struct sockaddr_in *addr)
{
    int fd = -1;
    char inputport[10];

    /* sets port string to 0 */
    memset(inputport,0,10);

    /* converts input port to the string port */
    sprintf(inputport,"%d", port);
    inputport[9]= '\0';

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port); /* Convert values between hos and network byte order */


    /*  Convert the Internet host address from IPv4 numbers-and-dots notation into
        binary form (network byte order) */
    if (inet_aton(ip, &(addr->sin_addr)) == 0)
    {
        fprintf(stderr,"[UDP CLIENT] Error: IP address is not valid\n\n");
        return -1;
    }
   
    fd = socket(AF_INET,SOCK_DGRAM, 0);
    if (fd == -1)
    {
        fprintf(stderr,"[UDP CLIENT] Error: Descriptor file not created in the socket\n");
        return -1;
    }

    return fd;
}

/********************************************************************************************************
 * EnviarUDP(struct sockaddr_in *addr, int fd, char *request, user_connection *user)
 * 
 * Parâmetros de Entrada:   addr -> estrutura para manipular endereços de internet
 *                          fd -> file descriptor a ser utilizado no envio da mensagem por ligação UDP
 *                          request -> mensagem a ser enviada por ligação UDP
 *                          user -> estrutura com parâmetros da aplicação iamroot e sobre as suas
 *                              conexões
 * 
 * Parâmetros de Saída:     int num_bytes -> retorna o número de bytes enviados pela ligação UDP
 *                                          estabelecida
 *                          caso contrário retorna -1
 * 
 * Descrição:   Envia uma mensagem por ligação UDP e retorna o número de bytes enviado. Se falhar,
 *              retorna -1.
 * 
 *******************************************************************************************************/
int EnviarUDP(struct sockaddr_in *addr, int fd, void *request,size_t dimensao, user_connection *user)
{
    int num_bytes = 0;
  	/* Função para transmissão de dados para um destino especificado */	
    num_bytes = sendto(fd, request, dimensao, MSG_CONFIRM, (const struct sockaddr *)addr, (socklen_t) sizeof(const struct sockaddr_in));
    
    if(num_bytes < 0 )
    {
        fprintf(stderr,"ERRO = sendto (EnviarUDP)\n");
    }
    if (user->debug)
    {
        fprintf(stderr,"[Mensagem UDP Enviada] = %p (%d bytes)\n",request,num_bytes);
    }
    return num_bytes;
}

/********************************************************************************************************
 * RecvUDP(struct sockaddr_in *addr, int fd, char *message, user_connection *user)
 * 
 * Parâmetros de Entrada:   addr -> estrutura para manipular endereços de internet
 *                          fd -> file descriptor a ser utilizado no envio da mensagem por ligação UDP
 *                          message -> mensagem recebida por ligação UDP
 *                          user -> estrutura com parâmetros da aplicação iamroot e sobre as suas
 *                              conexões
 * 
 * Parâmetros de Saida:     int nbytes -> retorna o número de bytes recebidos pela ligação UDP
 *                                  estabelecida
 * Descrição:   Recebe uma mensagem por ligação UDP a partir de um dado IP e porto
 * 
 *******************************************************************************************************/
int RecvUDP(struct sockaddr_in *addr, int fd, void *message,int dimensao, user_connection *user)
{
    int nbytes = 0;
    char *final=NULL;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    memset(message,0,dimensao);
    nbytes = recvfrom(fd, message, dimensao, MSG_CONFIRM, (struct sockaddr *) addr, &addrlen);
    if(nbytes < 0)
    {
        fprintf(stderr,"ERRO = Nao obteve a mensagem (RecvUDP)\n");
    } 
    if (sizeof(struct sockaddr_in)!=addrlen)
    {
        fprintf(stderr,"Nao corresponde ao mesmo endereco de socket\n");
        return -1;
    }
    if (nbytes == dimensao)
    {
    	final = message;
    	final[dimensao-1]='\0';
    } 

    if (user->debug)
    {
        fprintf(stderr,"Mensagem UDP  com %d bytes recebida\n", nbytes);
    }

    return nbytes;
}