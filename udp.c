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
 * Description:  Creation of a UDP server with the specified port at the entrance, returning a file
 *               descriptor.
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
 * Input parameters:        ip -> IP address which the UDP client does the connection
 *                          port -> port which the UDP client does the connection
 *                          addr -> structure for changing internet addresses
 * 
 * Output parameters:       int -> returns 0 if no errors, otherwise returns -1 
 * 
 * Description:   Does the UDP connection with a server
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


    /*  converts the Internet host address from IPv4 notation into
        binary */
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
 * int sendUDP(struct sockaddr_in *addr, int fd, void *request, size_t dim)
 * 
 * Input parameters:        addr -> structure for changing internet addresses
 *                          fd -> file descriptor used to send the message by udp
 *                          sendingMessage -> message to send by udp
 *                         
 * 
 * Output parameters:     int sentBytes -> returns the number of bytes sent by UDP
 *                        otherwise, it returns -1
 * 
 * Descrição:   This function sends a message by UDP. If it fails, it returns -1 instead of the number 
 *              of the bytes sent
 * 
 *******************************************************************************************************/
int sendUDP(struct sockaddr_in *addr, int fd, void *sendingMessage, size_t dim)
{
    int sentBytes = 0;

  	/* transimitting data */	
    sentBytes = sendto(fd, sendingMessage, dim, MSG_CONFIRM, (const struct sockaddr *)addr, (socklen_t) sizeof(const struct sockaddr_in));
    
    if(sentBytes < 0 )
        fprintf(stderr,"[UDP] Error: Message not sent \n");

    fprintf(stderr, "UDP Message sent with %d bytes\n", sentBytes);
   
    return sentBytes;
}

/********************************************************************************************************
 * RecvUDP(struct sockaddr_in *addr, int fd, char *message, user_connection *user)
 * 
 * Input parameters:        addr -> structure for changing internet addresses
 *                          fd -> file descriptor used to send the message by udp
 *                          recvMessageage -> received message by udp
 *                         
 * 
 * Output parameters:     int recvBytes -> returns the number of bytes received by UDP
 *                        otherwise, it returns -1
 * 
 * Description:   This function receives a message by UDP
 * 
 *******************************************************************************************************/
int ReceiveUDP(struct sockaddr_in *addr, int fd, void *recvMessage, int dim)
{
    int recvBytes = 0;
    char *message = NULL;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    memset(recvMessage, 0, dim);

    /* receiving data */	
    recvBytes = recvfrom(fd, recvMessage, dim, MSG_CONFIRM, (struct sockaddr *) addr, &addrlen);

    /* verification of reception*/
    if(recvBytes < 0)
        fprintf(stderr,"[UDP] Error: Message not received \n");
    
    if (sizeof(struct sockaddr_in) != addrlen)
    {
        fprintf(stderr,"[UDP] Error: The socket address is not the same \n");
        return -1;
    }
    if (recvBytes == dim)
    {
    	message = recvMessage;
    	message[dim-1]='\0';
    } 

    fprintf(stderr, "UDP Message received with %d bytes\n", recvBytes);

    return recvBytes;
}