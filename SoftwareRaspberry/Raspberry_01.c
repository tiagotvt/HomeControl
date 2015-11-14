//MASTER mac address: 78A5048C378C
//YELLOW mac address: 78A5048C47AF
//GREEN mac address:  78A5048C4A04


#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <wiringSerial.h>
//prototipos das funcoes
char* getstr(char* buff);
int connect(char* MAC);
int disconnect(void);
struct packet getpkt( void );
char checksum(char* data);
int send(char* idr, char*data);

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
//setup part
if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
{
   fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
   return 1 ;
}

int t;
int u1, u2;
char check[150];
char user[30];
char MAC[12];
struct packet pkin, pkout;

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

char* getstr(char* buff)
{
    int i = 0;
    int j = 0;

    while(!serialDataAvail(fd) && i<1000)
    {
        i++;
        delay(11);
    }

    if (i== 1000)
    {
        printf("sem dados no serial\n");
        buff = "erro";
        return(buff);
    }

    while(serialDataAvail(fd)>0)
    {
        delay(10);
        buff[j] = serialGetchar(fd);
        j++;
    }
    buff[j] = 0;

    printf("%s \n", buff); //debug

    return(buff);
}

int connect(char* MAC)
{
    char check[10];
    char buff[20];
    int err;

    serialPuts (fd,"AT+ROLE1");
    getstr(check);
    delay(800);

	buff[0] = '\0';
	strcat(buff,"AT+CON");
	strcat(buff,MAC);
	puts(buff);

    for(err=0; err<3; err++)
    {
        //serialPuts (fd,"AT+CON");
        //serialPuts (fd,MAC);
	serialPuts (fd,buff);
	//serialPuts (fd,"AT+CON78A5048C4A04");
        getstr(check);

        if (strcmp(check,"OK+CONNA") == 0)
        {
        check[0] = 0;
        printf("comando recebido\n"); //debug
        }
        else return(2); //codigo de erro para comando nao entendido

        getstr(check);

        if ((strcmp(check,"OK+CONN") == 0)) //verifica se a conexao foi efetuada
        {
            check[0] = 0;
            printf("conectou!\n");
            return(0); //retorno 0 indica que a conexao foi feita com sucesso
        }
    }


    check[0] = 0;
    printf("conexao falhou!\n");
    return (1); //retorno 1 indica que a conexao nao pode ser efetuada

}

int disconnect()
{
    char check[30];
    serialPuts (fd,"AT");
    getstr(check);

    if ((strcmp(check,"OK+LOST") == 0))
    {
        check[0] = 0;
        serialPuts (fd,"AT+ROLE0");
        getstr(check);
        return(0); //retorno 0 indica que a desconxao foi feita
    }

    else if ((strcmp(check,"OK") == 0))
    {
        check[0] = 0;
        serialPuts (fd,"AT+ROLE0");
        getstr(check);
        return(3); //retorno 3 indica que nao havia conexao para ser desfeita
    }
}

struct packet getpkt( void )
{
    int i = 0;
    int j = 0;
    int z;
    char buff[193];
    struct packet ret;

    for (z=0; z<3; z++)
    {
        while(!serialDataAvail(fd) && i<1000)
        {
            i++;
            delay(10);
        }

        if (i== 1000)
        {
            printf("sem dados no serial\n");
            ret.ids[0] = 0;
            ret.idr[0] = 0;
            strcpy(ret.data,"erro");
            ret.cs = 0;
            return(ret);
        }

        while(serialDataAvail(fd)>0)
        {
            delay(10);
            buff[j] = serialGetchar(fd);
            j++;
        }
        buff[j] = '\0';

        printf("%s \n", buff); //debug

        strncpy(ret.ids,buff,12);
        ret.ids[12] = '\0';
        strncpy(ret.idr,buff+12,12);
        ret.idr[12] = '\0';
        strncpy(ret.data,buff+24,(j-25));
        ret.data[j-25] = '\0';
        ret.cs = buff[j-1];

        if (strcmp(ret.data,"CSOK") == 0) return(ret); //excecao para impedir um loop infinito de confirmacao
        if (strcmp(ret.data,"CSFAIL") == 0) return(ret); //excecao para impedir um loop infinito de confirmacao

        if (ret.cs == checksum(ret.data))
        {
            send(ret.idr,"CSOK");
            return(ret);
        }
    }
    return(errorpkt);
}

char checksum(char* data)
{
    char cs = 0;
    int i = 0;
    int j = strlen(data);
    while(i < j)
    {
        cs += data[i];
        i++;
    }
    return(cs);
}

int send(char* idr, char* data)
{
    char* ids = "78A5048C378C"; //MAC do modulo da RPi
    char buff[191];
         buff[0] = '\0';
    char cs[2];
        cs[0] = checksum(data);
        cs[1] = '\0';
    struct packet temp;
    int z;

    strcat(buff,ids);
    strcat(buff,idr);
    strcat(buff,data);
    strcat(buff,cs);

    for (z=0; z<3; z++)
    {
        serialPuts (fd,buff);
        if (strcmp(data,"CSOK") == 0) return(0); //excecao para impedir um loop infinito de confirmacao
        if (strcmp(data,"CSFAIL") == 0) return(0); //excecao para impedir um loop infinito de confirmacao
        temp = getpkt();
        if (strcmp(temp.data,"CSOK") == 0)
        {
            return(0);
        }
    }
    return(1);
}



