#include <SoftwareSerial.h>
#include <Arduino.h>
SoftwareSerial mySerial(11, 10); // RX, TX

struct packet
{
  String ids;
  String idr;
  String data;
  char cs;
};

//function prototypes
packet getpkt(void);
char checksum(String);
int send(String, String);
String getstr(void);
int connect(String);
int disconnect(void);
int bounce(packet);

const packet errorpkt = {"000000000000", "000000000000", "error", 0};

packet pkt;
String check = "";

String idlocal = "78A5048C47AF";


//------------------------------------------------------CHECKSUM---------------------------------------------------------------

char checksum(String data)
{
  char checksum = 0;
  int i = 0;
  while (i < data.length())
  {
    checksum = checksum + data.charAt(i);
    i++;
  }
  return (checksum);
}

//------------------------------------------------------END CHECKSUM-----------------------------------------------------------
//------------------------------------------------------SEND-------------------------------------------------------------------

int send(String idr, String data)
{
  //colocar o mac do modulo que esta sendo gravado aqui
  //String ids = "78A5048C4A04";  //green
  //String ids = "78A5048C47AF";  //yellow

  packet temp;
  String buff = "";
  char cs = checksum(data);
  buff = idlocal + idr + data + cs;

  for (int i = 0; i < 3; i++)
  {
    mySerial.print(buff);
    if (data == "CSOK") return (0); //excecao para impedir um loop infinito de confirmacao
    if (data == "CSFAIL") return (0); //excecao para impedir um loop infinito de confirmacao
    temp = getpkt();
    if (temp.data == "CSOK") return (0);
  }
  return (1);
}

//------------------------------------------------------END SEND---------------------------------------------------------------
//------------------------------------------------------GETPACKET--------------------------------------------------------------

packet getpkt()
{
  char c;
  String buff = "";
  packet ret;
  int l;
  int i = 0;

  //mySerial.flush();

  for (int z = 0; z < 3; z++)
  {
    while (!mySerial.available() && i < 1000 ) //espera a informacao comecar a chegar. Se demorar mais de 10s cancela
    {
      i++;
      delay(10);
    }

    if (i == 1000)
    {
      Serial.write("sem dados no serial\n");
      return (errorpkt);
    }

    while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string buff
    {
      c = mySerial.read();
      buff += c;
      delay(10);  //sem esse delay o while acaba antes de todos os dados passarem
    }
    Serial.print(buff); //debug

    l = buff.length();
    if (l<24) 
    {
      ret = errorpkt;
      ret.data = buff;
      return(ret);
    }
    ret.ids = buff.substring(0, 12);
    ret.idr = buff.substring(12, 24);
    ret.data = buff.substring(24, l - 1);
    ret.cs = buff.charAt(l - 1);

    if (ret.data == "CSOK") return (ret); //excecao para impedir um loop infinito de confirmacao
    if (ret.data == "CSFAIL") return (ret); //excecao para impedir um loop infinito de confirmacao
    if (ret.idr != idlocal) //definir o MAC do módulo que está sendo gravado!
    {
       l = bounce(ret);
       if (l == 0) return(errorpkt);
    }

    if (ret.cs == checksum(ret.data))
    {
      send(ret.ids, "CSOK");
      Serial.write("OK!\n"); //debug
      return (ret);
    }
    else send(ret.ids, "CSFAIL");
  }
  return (errorpkt);
}

//------------------------------------------------------END GETPACKET----------------------------------------------------------
//------------------------------------------------------GETSTRING--------------------------------------------------------------
String getstr()
{
  char c;
  String buff = "";
  int i = 0;

  //mySerial.flush();

  while (!mySerial.available() && i < 1000 ) //espera a informacao comecar a chegar. Se demorar mais de 10s cancela
  {
    i++;
    delay(10);
  }

  if (i == 1000)
  {
    Serial.write("sem dados no serial\n");
    return ("0");
  }

  while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string buff
  {
    c = mySerial.read();
    buff += c;
    delay(10);  //sem esse delay o while acaba antes de todos os dados passarem
  }

  Serial.print(buff); //debug
  Serial.print("\n"); //debug

  return (buff);
}

//------------------------------------------------------END GETSTRING----------------------------------------------------------
//------------------------------------------------------CONNECT----------------------------------------------------------------
int connect(String MAC)
{
  mySerial.write("AT+ROLE1");  //coloca o modulo em modo master para se conectar a outro
  delay(800);
  pkt = getpkt();

  int err;
  MAC = "AT+CON" + MAC;
  for (err = 0; err < 3; err++)
  {
    mySerial.print(MAC);  //conecta com o modulo especificado pelo mac

//    Serial.print(MAC);
    pkt = getpkt();

    if (pkt.data == "OK+CONNA") //se verificado que o módulo entendeu o comando, limpa a string e prossegue
    {
      pkt.data.remove(0); //se o ble entendeu o comando, limpa a string e prossegue
    }
    else return (-1); //se nao, retornar erro desconhecido

    pkt = getpkt();

    if (pkt.data == "OK+CONN")    //verifica se a conexao foi efetuada
    {
      pkt.data.remove(0);
      return (1);
    }
  }

  pkt.data.remove(0);
  return (0);

}

//------------------------------------------------------END CONNECT------------------------------------------------------------
//------------------------------------------------------DISCONNECT-------------------------------------------------------------

int disconnect()
{
  mySerial.write("AT");  //Se o modulo estiver conectado, AT termina a conexao

  check = getstr();

  if (check == "OK+LOST")
  {
    delay(500);
    mySerial.write("AT+ROLE0");  //coloca o modulo em modo slave para ficar disponivel a receber conexoes
    check = getstr();
    return (0);
  }
  else if (check == "OK")
  {
    delay(500);
    check.remove(0);
    mySerial.write("AT+ROLE0");  //coloca o modulo em modo slave para ficar disponivel a receber conexoes
    check = getstr();
    return (-1);
  }
  return(-2);
}

//------------------------------------------------------END DISCONNECT---------------------------------------------------------

//------------------------------------------------------BOUNCE-------------------------------------------------------------
int bounce(packet foward)
{
  packet reply;

  Serial.write("\nmark 1\n");
  disconnect();
  Serial.write("\nmark 2\n");
  connect(foward.idr);
  Serial.write("\nmark 3\n");
  send(foward.idr, foward.data);
  Serial.write("\nmark 4\n");
  reply = getpkt();
  Serial.write("\nmark 5\n");
  delay(500);
  disconnect();
  Serial.write("\nmark 6\n");
  connect(foward.ids);
  Serial.write("\nmark 7\n");
  delay(2000);  //DIMINUIR ESSE DELAY - ESTA ALTO PARA PERMITIR QUE A RASP SEJA COLOCADA EM GETPKT MANUALMENTE
  send(foward.ids,reply.data);
  Serial.write("\nmark 8\n");
  return(0);
}

//------------------------------------------------------END BOUNCE-------------------------------------------------------------



