/***********************************************************************
 * Project: Name Based Networks
 *
 * Subject: Computer Networks and the Internet
 * 2nd Semester 2020/2021
 *
 * Authors:
 * 				Duarte Oliveira, nº 94192
 * 				Diogo Martins,nº 93047
 *
 * Laboratory Shift: Monday, 8h-10h
 *
 * Last modification: 25-03-2021
 * Filename: ndn.c
 *
 * Comments: Desenvolvimento de uma aplicação (iamroot), com a qual um 
 * conjunto de pares mantêm uma rede em árvore, construída à base de sessões 
 * TCP estabelecidas entre eles, a qual é usada para a disseminação de um 
 * conteúdo stream disponibilizado por um servidor TCP (fonte) 
 **********************************************************************/

/***************************
 **      libraries      **
 ***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include "structs.h"



/*********************************************************************************************
 * main (int argc, char *argv[])
 *
 * Parâmetros de Entrada:   argc -> número de argumentos da linha de comandos
 * 	                        argv -> tabela de ponteiros para argumentos do tipo string
 * Parâmetros de Saída:     int -> retorna 0 se for bem sucedido, 1 se for ocorrer um erro
 *
 * Descrição: Função principal da aplicação iamroot (main)
 * 
 *********************************************************************************************/
int main (int argc, char *argv[])
{
    /*********************************************************
     **                      VARIAVEIS                      **
     *********************************************************/
    int i = 0, j = 0, n = 0, aux = 0; /* variaveis auxiliares */ 	
    int fd = -1; /* file descritptors */
	int num_bytes = 0; /* numero de bytes das mensagens recebidas e enviadas */
	int dimensao1 = 0, dimensao2 = 0; /*dimensao das palavras vindas do pai e dos filhos */

    /* Strings de armazenamento temporário de informacoes */
 	char pedido[5000], resposta[5000];

    user_connection *user = NULL; /* ponteiro para estrutura dos dados do utilizador e aplicacao */
    listpops *head = NULL; /* ponteiros para a lista de pops */

    /* Flags de controlo de estado (caso haja algum problema termina) e de adesao à árvore */
    enum {conectado, desconectado}adesao;
    enum {continuar, terminar}estado;

	struct timeval temporizador; /*Estrutura timeval para definir o tempo que a raiz tem de refrescar o Serv. de raiz */
    fd_set rfds; /* estrutura que contêm todos os fd's usados */
    int counter = 0; /* valor retornado pelo select que traduz o numero de fds, counter é 0 quando o temporizador expira */
    int maxfd = 0; /* valor maximo do fd para introduzir no select */

    int extra = 0; /* variavel de controlo */

    
    signal(SIGPIPE,SIG_IGN);

    /*Limpeza dos buffers do lixo que possam contem */
    memset(pedido,0,5000);
    memset(resposta,0,5000);

    /******************************************************
     **                     INICIO                       **
     ******************************************************/

    /* No início a aplicação está desconectada da árvore */
    estado = continuar;   
    adesao = desconectado;
    
    /* Faz a leitura de parâmetros de entrada a aplicação */
    if(leitura_parametros(&user, argc, argv) == -1)
    {
        fprintf(stderr,"ERRO = Houve problemas com a leitura de parâmetros\n");
        if(LibertaMemoria(user, head) == -1)
        {
            fprintf(stderr,"Erro na libertação de memória\n");

            exit(1);
        }
        exit(0);
    }

    /*Print dos parametros de entrada alojados na estrutura criada caso o debug esteja ativo */
    if (user->debug)
	{
		printf("\n-------PARAMETROS----------\n");
		printf("Stream=%s\nIPfonte=%s\nPortoFonte=%d\n", user->nome_stream, user->ip_stream, user->porto_stream);
		printf("ipaddr=%s\n", user->ipaddr);
		printf("tport=%d\nuport=%d\n", user->tport, user->uport);
		printf("rsaddr=%s||rsport=%d\n", user->rsaddr, user->rsport);
		printf("tcpsessions=%d\nbestpops=%d\ntsecs=%d\n", user->tcpsessions, user->bestpops, user->tsecs);
		printf("Flags: b = %d | d = %d | h = %d\n\n\n", user->display, user->debug, user->help);
	}

    if (estado == continuar)
    {
        /* Alocacao de memoria para os parametros de armazenamento de dados : 
           lista de pops, informacoes do pai e do filhos (etc...) */       
        if(AlocacaoMemoria(user))
        {
            fprintf(stderr,"Erro na alocação de memória\n");
            estado = terminar;
        }


        /* Cria, liga e coloca a socket à espera dos filhos */    
        user->fd_recv = ServerTCP(user->tport); 
        if (user->fd_recv == -1)
        {
            fprintf(stderr,"ERRO = Falha na abertura da socket do protocolo TCP\n");
            estado = terminar;
        }

        /* O temporizador armazena o periodo associado ao registo periodico que a raiz deve fazer no 
           Servidor de Raizes */
        temporizador.tv_sec = user->tsecs;
        temporizador.tv_usec = 0;
    
    }

    /* Loop principal da aplicação */
    while(estado == continuar)
    {
        /* Adesao à árvore de escoamento no inicio ou na interrupcao e restabelecimento do stream */        
        if (adesao == desconectado)
        {
    
            sleep(2); /* Antes de inicirar o resto espera 2 segundos para que tudo esteja bem */
            printf("\nCONECTANDO-SE À STREAM ....\n");

            /* Mensagem protocolar enviada ao servidor de raizes pedindo o IP e o porto UDP  do servidor
               de acesso da raiz, caso já haja */           
            j = sprintf(pedido,"WHOISROOT %s %s:%d\n", user->streamID, user->ipaddr, user->uport);
            if ( j < 0)
            {
                fprintf(stderr,"Nao foi formulado o pedido WHOISROOT\n");
                estado = terminar;
            }
            /* Comunica com o Servidor de Raízes para perguntar quem é a raiz do stream especificado,
            ou então faz o registo da raiz, se ainda não houver raiz */
            if(EnvioServRaizes(pedido, resposta, user) == -1)
            {
                fprintf(stderr,"ERRO = No envio do pedido WHOISROOT ao servidor de raizes\n");
                estado = terminar;
            }
            
            /* Comunica com o Servidor de Raízes para receber uma resposta, dizendo quem é a raiz */
            if( estado == terminar || RecebServRaizes(resposta, user) == -1)
            {
                fprintf(stderr,"ERRO = Nao recebeu a resposta do servidor de raizes\n");
                estado = terminar;
                break;
            }

            if (estado == continuar)
            {
   
                /* Par fica conectado ao stream */
                adesao = conectado;
                Estado(user);
                graph_user();
            }
        }

        /* Monta todos os file descriptors necessários ao bom funcionamento da aplicação */
        FD_ZERO(&rfds);

       	maxfd=2; /* o maxfd começa a 2 porque o fd = 0 e 1 estao reservados para o sistema */
        

        /* Se for a raiz faz FD_SET do file descriptor de conexao ao servidor de acesso */
        if(user->raiz)
        {
            FD_SET(user->fd_acesso, &rfds);
            if(user->fd_acesso >= maxfd)
            {
            	maxfd = max(maxfd,user->fd_acesso);
            }
        }


        /*FD_SET DO STDIN */
        FD_SET(0, &rfds); 

        /* Só faz FD_SET  do file descriptor pai quando a aplicacao nao esta interrompida */
       /* if (user->interrompido == 0)
        {*/
    
            FD_SET(user->fd_montante, &rfds);
	        if(user->fd_montante >= maxfd)
	        {
	            maxfd = max(maxfd,user->fd_montante);
	        }
	    /*}*/


		FD_SET(user->fd_recv, &rfds);
	    if(user->fd_recv >= maxfd)
	    {
	        maxfd = max(maxfd,user->fd_recv);
	    }

	    /* FD_SET dos files descriptors dos filhos */
	 	for(aux = 0; aux < user->numfilhos; aux++)
		{
		    FD_SET(user->fd_jusante[aux], &rfds);
			if (user->fd_jusante[aux] >= maxfd)
		    {
		        maxfd = max(maxfd,user->fd_jusante[aux]);
		    }
        }

       
        /* Lança o PopQuery caso o número de pontos de acesso seja menor que o definido no início da
        invocação, para encontrar mais pontos de acesso */
        if(user->raiz && user->contadorPops < user->bestpops)
        {
            /* Se houver ligações TCP disponíveis na raiz, adiciona esses pontos de acesso à lista */
            if(user->tcpsessions > user->numfilhos)
            {
                if(sprintf(user->PopAddr[user->contadorPops],"%s:%d", user->ipaddr, user->tport) < 0)
                {
                    fprintf(stderr,"ERRO = Na colocação dos parâmetros no PopAddr\n");
                }
                else
                {
                    user->contadorPops++;
                }
            }
            /* Dado só querermos enviar os primeiros <bestpops> a montante definimos um valor de 1/2 */
            if((user->bestpops)/2 > user->contadorPops)
            {
                if(Pop_Query(user) == -1)
                {
                    fprintf(stderr,"ERRO = Falha no envio dos Pop Queries\n");
                    estado = terminar;
                    break;
                }

                sleep(1);
            }
        }

        /***********************************************************************************************
         * 
         * Código para o Select -> indica quais são os file descriptors prontos a serem lidos/escritos
         * 
         * *********************************************************************************************/
        counter = select(maxfd+1, &rfds, NULL, NULL, &temporizador);
        /* se houver algum erro com o select o counter é negativo */
        if(counter < 0)
        {
            fprintf(stderr,"ERRO = O counter do select deu negativo\n");
            break;
        }
        /* Age de acordo com o facto de não haver nenhum file descriptor pronto para ser lido/escrito */
        else if(counter == 0)
        {
            /*Quando o temporizador chega ao fim faz refresh na raiz */
            if(user->raiz && (RefrescaRaiz(user) < 0 )) 
            {
   
                fprintf(stderr,"ERRO = Não fez refresh à raiz\n");
                break;
            }

            /* Se o stream estiver interrompido, a aplicação tenta recuperar a ligação à árvore de
            escoamento */
            if(user->interrompido)
            {
                close(user->fd_montante);
                adesao = desconectado;               
            }

            /* Actualiza o temporizador  */
            temporizador.tv_sec = user->tsecs;
            temporizador.tv_usec = 0;
            continue;
        }
        /* Verifica quais são os file descriptors que estão prontos a serem lidos/escritos */
        else
        {
            /* Verifica file descriptor do input do utilizador */
            if(FD_ISSET(0, &rfds))
            {
                if(fgets(resposta, 5000, stdin) == NULL)
                {
                    fprintf(stderr,"ERRO = Tamanho da palavra da interface do utilizador > 30 caracteres...\n");
                    break;
                }

                aux = interface_utilizador(user, resposta);
                if(aux == 1)
                {
                    estado = terminar;
                }
                continue;                    
            }

            /* Verifica file descriptor da ligação TCP a montante */
            if(FD_ISSET(user->fd_montante, &rfds))
            {
   
                memset(resposta,0,5000);
                num_bytes = ReceiveTCP(user->fd_montante,resposta,5000,user);
                if (num_bytes == 0)
                {

                    /* Após verificação de que não foi recebido nada, assinala que a ligação com o pai
                    foi interrompida */
                    close(user->fd_montante);
                    user->interrompido = 1;
                    /* Mensagem protocolar enviada ao servidor de raizes pedindo o IP e o porto UDP  do servidor
                    de acesso da raiz, caso já haja */           
                    j = sprintf(pedido,"WHOISROOT %s %s:%d\n", user->streamID, user->ipaddr, user->uport);
                    if ( j < 0)
                    {
                        fprintf(stderr,"Nao foi formulado o pedido WHOISROOT\n");
                        estado = terminar;
                    }
                    /* Comunica com o Servidor de Raízes para perguntar quem é a raiz do stream especificado,
                    ou então faz o registo da raiz, se ainda não houver raiz */
                    if(EnvioServRaizes(pedido, resposta, user) == -1)
                    {
                        fprintf(stderr,"ERRO = No envio do pedido WHOISROOT ao servidor de raizes\n");
                        estado = terminar;
                    }
                    
                    /* Comunica com o Servidor de Raízes para receber uma resposta, dizendo quem é a raiz */
                    if( estado == terminar || RecebServRaizes(resposta, user) == -1)
                    {
                        fprintf(stderr,"ERRO = Nao recebeu a resposta do servidor de raizes\n");
                        estado = terminar;
                        break;
                    }
                    /* Envia a mensagem aos filhos */
                    Estado(user);
                    graph_user();
                    continue;
                    
                }
                else if(num_bytes < 0)
                {
                    
                    fprintf(stderr,"ERRO = Houve falha na conexão TCP com o pai\n");
                    estado = terminar;

                }
                else
                {
                	dimensao1 = 0;
                	dimensao2 = strlen(user->info_montante);

                    /* Vai percorrer todo o buffer da resposta vinda do pai para armazenar num buffer
                        da estrutura e para enviar aos filhos */
                    while(dimensao1 < (int)strlen(resposta))
                    {
                        if(resposta[dimensao1] != '\n' && dimensao2 < 499)
                        {
                            user->info_montante[dimensao2] = resposta[dimensao1];
                            dimensao2++;
                        }
                        else
                        {
                            user->info_montante[dimensao2] = resposta[dimensao1];
                            dimensao2++;
                            if ( resposta[dimensao1] != '\n')
                                user->info_montante[dimensao2]='\n';
                            /* Espalha a mensagem do pai para os filhos */
                            extra = DisseminaJus(user,&head,user->info_montante,extra);
                            if(extra <0)
                            {
                                fprintf(stderr,"ERRO = Não conseguiu propagar a mensagem do pai a jusante\n");
                                estado = terminar;
                                break;
                            }

                            dimensao2 = 0;

                            memset(user->info_montante, 0, 500);
                        }

                        dimensao1++;
                    }
                }              
            }

            /* Verifica se ha alguem a tentar juntar-se à arvore */
            if(FD_ISSET(user->fd_recv, &rfds))
            {

                if(user->tcpsessions > user->numfilhos)
                {

                    if((user->fd_jusante[user->numfilhos] = AcceptTCP(user->fd_recv, NULL)) == -1)
                    {
                        fprintf(stderr,"ERRO = Não aceitou o filho\n");
                    }
                    else
                    {
                        if(Welcome(user->numfilhos,user) == -1)
                        {
                            fprintf(stderr,"ERRO = Não enviou Welcome ao Filho\n");
                            close(user->fd_jusante[user->numfilhos]);
                            user->numfilhos--;
                        }
                        user->numfilhos++;
                    }                   
                }
                else
                {
                    /* Aceita o novo fd */
                    fd = AcceptTCP(user->fd_recv,NULL);
                    if(fd == -1)
                    {
                        fprintf(stderr,"ERRO = Não aceitou o filho\n");
                    }

                    if(user->numfilhos <= 0)
                    {
                        fprintf(stderr,"ERRO = Não há filhos para fazer REDIRECT\n");
                    }
                    else
                    {
                        /* Faz Redirect para o novo ponto */
                        if (Redirect(fd, user->FilhosAddr[user->numfilhos-1], user) == -1)
                        {
                            fprintf(stderr,"ERRO = Não conseguiu fazer REDIRECT\n");
                        }
                    }
                    close(fd);
                }
            }

            /* Corre todos os file descriptors dos filhos que foram selecionados */
            for(aux = 0; aux < user->numfilhos; aux++)
            {
                if(FD_ISSET(user->fd_jusante[aux], &rfds))
                {
    
                    memset(resposta,0,5000); /* Limpeza do buffer */
                    /*Recebe resposta dos filhos */
                    num_bytes= ReceiveTCP(user->fd_jusante[aux],resposta,5000,user);
                    if (num_bytes == 0)
                    {
   
                        close(user->fd_jusante[aux]); /* Fecha o fd */
                        user->interrompido = 1; /* Coloca como interrompido */

                        /* Atualiza os filhos dada a saída de 1 */
                        for(n = aux; n < user->numfilhos-1;n++)
                        {
                            user->fd_jusante[n] = user->fd_jusante[n + 1];
                            if(user->FilhosAddr[0] != NULL)
                            {
                                strcpy(user->FilhosAddr[n], user->FilhosAddr[n + 1]);
                            }
                        }
            
                        user->numfilhos--; /* Decrementa dado já não existir nenhum */
                        break;                     

                    }
                    else if(num_bytes < 0)
                    {

                        estado = terminar;
                        fprintf(stderr,"ERRO = O filho não se conectou por TCP\n");
                        break;
                    }
                    else
                    {
                        /* Copiar a mensagem vinda dos filhos */
                        dimensao1 = 0;
                        dimensao2 = strlen(user->info_jusante[aux]);

                        while( dimensao1 < (int)strlen(resposta))
                        {
                            if(resposta[dimensao1] != '\n' && dimensao2 < 5000)
                            {
                                user->info_jusante[aux][dimensao2] = resposta[dimensao1];
                                dimensao2++;
                            }
                            else
                            {
                                user->info_jusante[aux][dimensao2] = resposta[dimensao1];
                                dimensao2++;

                                /*Propaga a mensagem para o pai */
                                if((user->filhoextra[aux] = DisseminaMon(user->info_jusante[aux], user, aux, &head)) < 0)
                                {
                                    user->filhoextra[aux] = 0;   
                                }
                                if(user->filhoextra[aux] == 0)
                                {
                                    dimensao2 = 0;
                                    memset(user->info_jusante[aux], 0, 5000);

                                }
                                else if(dimensao2 > 4998)
                                {
                                    dimensao2 = 0;
                                    memset(user->info_jusante[aux], 0, 5000);
                                }

                            }
                            dimensao1++;   
                        }
                    }               
                }
            }

            /* Se eu for a raiz lê o fd de acesso */
            if(user->raiz && FD_ISSET(user->fd_acesso, &rfds))
            {

                user->contadorPops--; 

                if (user->contadorPops < 0)
                {
   
                    user->contadorPops = 0;
                    sleep(2);
                }
                else
                {
                    /*Pop Request*/
                    PopResponde(user->PopAddr[user->contadorPops], user->fd_acesso, user);
                } 
            }
        }  
          
    } 

    if (user->raiz)
    {

        sprintf(pedido,"REMOVE %s\n",user->streamID);
        if ( EnvioServRaizes(pedido,resposta,user))
        {
            fprintf(stderr,"ERRO = No envio do REMOVE\n");
        }
    }
    else
    {
        printf("ROOT desligou-se...\n");
    }
    



    if (user->fd_acesso != -1) 
        close(user->fd_acesso);

    if (user->fd_montante != -1) 
        close(user->fd_montante);

    for(i=0;i<user->numfilhos;i++) 
        close(user->fd_jusante[i]);

    if (user->fd_recv != -1)
        close(user->fd_recv);

    

    if(LibertaMemoria(user, head) == -1)
    {
        fprintf(stderr,"Erro na libertação de memória\n");

        exit(1);
    }

    return 0;
}
