//MASTER mac address: 78A5048C378C
//YELLOW mac address: 78A5048C47AF
//GREEN mac address:  78A5048C4A04


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "hm10.h"
//#include <wiringSerial.h>
//prototipos das funcoes


int fd ;
struct packet
{
  char ids[13];
  char idr[13];
  char data[150];
  char cs;
};

const struct packet errorpkt = {"000000000000","000000000000","error",0};

int main ()
{


//loop part
while(1)
{
    puts("Selecione a operacao desejada:");
    puts("1: conectar a outra unidade");
    puts("2: desconectar a unidade");
    puts("3: enviar comando");
    puts("4: enviar dados");
    puts("5: receber dados");
    scanf("%i", &u1); //recebe o comando do usuario (1 ou 2)
    switch(u1)
    {
    case 1:
        puts("Com qual unidade deseja se conectar?");
        puts("1: Green");
        puts("2: Yellow");
        scanf("%i", &u2);
        switch(u2)
        {
        case 1:
            connect("78A5048C4A04");
            break;
        case 2:
            connect("78A5048C47AF");
            break;
        default:
            puts("opção inválida!");
            break;
        }
        break;
    case 2:
        t = disconnect();
        if (t==0) puts("desconectado!");
        else if (t==3) puts("ja estava desconectado!");
        break;
    case 3:
        puts("Digite o comando que quer enviar");
        scanf("%s",user);
        serialPuts (fd, user);
        delay(100);
        getstr(check);
        break;
    case 4:
        puts("Digite a mensagem que quer enviar");
        scanf("%s",user);
        //serialPuts (fd, user);
        send("78A5048C4A04", user);
        break;
    case 5:
        puts("Recebendo");
        pkin = getpkt();
        printf("%s \n", pkin.data);
        break;
    default:
        puts("opção inválida!");
        break;
    }
    delay(200);
    serialFlush (fd);
}

}


