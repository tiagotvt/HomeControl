/*
 * serialRead.c:
 *	Example program to read bytes from the Serial line
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

//#include <wiringSerial.h>

int main ()
{
  int fd ;
  int b = 0;
  int a = 0;
  int i = 0;
  char mac[] = "78A5048C47AF"; //green module mac
  char check[20] = "\0";

  if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  fflush(stdout);
  serialPuts (fd,"AT+MODE1"); //serve para acordar o modulo
  delay(500);
  serialFlush (fd) ;
  printf("go\n");



    serialPuts (fd,"AT+CON");
    serialPuts (fd,mac);

    while(serialDataAvail(fd)==0); //espera a chegada de algum byte na porta serial

    //o bloco abaixo recebe as informacoes do buffer e copia para a string check
    i=0;
    while((serialDataAvail(fd))!=0)
    {
    delay(10);
    check[i] = serialGetchar(fd);
    i++;
    }
    check[i] = 0;
    //fim do bloco que copia os dados para a string check

    printf("%s \n", check); //debug

    if (strcmp(check,"OK+CONNA") == 0) //se verificado que o módulo entendeu o comando, limpa a string e prossegue
    {
    check[0] = 0; //se o ble entendeu o comando, limpa a string e prossegue
    printf("comando recebido\n"); //debug
    }
    else
    {
    if (strcmp(check,"OK+CONNAOK+CONN") == 0)
    {
    check[0] = 0;
    printf("conectou!\n"); //para o caso da conexão ser estabelecida entre uma tentativa e outra
    }
    else return printf("erro!\n"); //se nao, retornar erro desconhecido
    }


    while(serialDataAvail(fd)==0); //espera a segunda parte da mensagem

    //o bloco abaixo recebe as informacoes do buffer e copia para a string check
    i=0;
    while((serialDataAvail(fd))!=0)
    {
    delay(10);
    check[i] = serialGetchar(fd);
    i++;
    }
    check[i] = 0;
    //fim do bloco que copia os dados para a string check

    if ((strcmp(check,"OK+CONN") == 0))    //verifica se a conexao foi efetuada
    {
        check[0] = 0;
        printf("conectou!\n");
    }
    else
    {
        check[0] = 0;
        printf("conexao falhou!\n");
    }
}
