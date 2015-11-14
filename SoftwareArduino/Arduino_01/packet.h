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
int send(String,String);
String getstr(void);
int connect(String);
int disconnect(void);

const packet errorpkt = {"000000000000","000000000000","error",0};

String check = "";


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
  return(checksum);
}

//------------------------------------------------------END CHECKSUM-----------------------------------------------------------
//------------------------------------------------------SEND-------------------------------------------------------------------

int send(String idr, String data)
{
//colocar o mac do modulo que esta sendo gravado aqui
String ids = "78A5048C4A04";  //green
//String ids = "78A5048C47AF";  //yellow

    packet temp;
    String buff = "";
    char cs = checksum(data);
    buff = ids + idr + data + cs;

    for (int i=0; i<3; i++)
      {    
      mySerial.print(buff);
      if (data == "CSOK") return(0); //excecao para impedir um loop infinito de confirmacao
      if (data == "CSFAIL") return(0); //excecao para impedir um loop infinito de confirmacao
      temp = getpkt();
      if (temp.data == "CSOK") return(0);
      }
    return(1);
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


      for (int z=0; z<3; z++)
      {
        while(!mySerial.available() && i<1000 ) //espera a informacao comecar a chegar. Se demorar mais de 10s cancela
        {
            i++;
            delay(10);
        }
  
        if (i == 1000)
        {
            Serial.write("sem dados no serial\n");
            return(errorpkt);
        }
      
        while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string buff
        {
            c = mySerial.read();
            buff += c;
            delay(10);  //sem esse delay o while acaba antes de todos os dados passarem
        }
        
        l = buff.length();
        ret.ids = buff.substring(0,12);
        ret.idr = buff.substring(12,24);
        ret.data = buff.substring(24,l-1);
        ret.cs = buff.charAt(l-1);

        if (ret.data == "CSOK") return (ret); //excecao para impedir um loop infinito de confirmacao
        if (ret.data == "CSFAIL") return (ret); //excecao para impedir um loop infinito de confirmacao
  
        if (ret.cs == checksum(ret.data))
        {
          send(ret.ids,"CSOK");
          return (ret);
        }
        else send(ret.idr,"CSFAIL");
      }
      return(errorpkt);
}

//------------------------------------------------------END GETPACKET----------------------------------------------------------
//------------------------------------------------------GETSTRING--------------------------------------------------------------
String getstr()
{
      char c;
      String buff = "";
      int i = 0;
      
      while(!mySerial.available() && i<1000 ) //espera a informacao comecar a chegar. Se demorar mais de 10s cancela
      {
          i++;
          delay(10);
      }

      if (i == 1000)
      {
          Serial.write("sem dados no serial\n");
          return("0");
      }
  
      while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string buff
      {
          c = mySerial.read();
          buff += c;
          delay(10);  //sem esse delay o while acaba antes de todos os dados passarem
      }
      
      Serial.print(buff); //debug
      Serial.print("\n"); //debug

      return(buff);
}
      
//------------------------------------------------------END GETSTRING----------------------------------------------------------
//------------------------------------------------------CONNECT----------------------------------------------------------------
int connect(String MAC)
{
          mySerial.write("AT+ROLE1");  //coloca o modulo em modo master para se conectar a outro 
          delay(800);
          check = getstr();

          int err;
          MAC = "AT+CON" + MAC;
          for(err = 0; err<3; err++)
          {        
            //conecta com o modulo especificado pelo mac
            //mySerial.print("AT+CON");
            mySerial.print(MAC);

            Serial.print(MAC);            
            check = getstr();
           
            if (check == "OK+CONNA") //se verificado que o módulo entendeu o comando, limpa a string e prossegue
            {
            check.remove(0); //se o ble entendeu o comando, limpa a string e prossegue
            }
            else return (-1); //se nao, retornar erro desconhecido
            
            check = getstr();
            
            if (check == "OK+CONN")    //verifica se a conexao foi efetuada 
            {
            check.remove(0);
            return (1);
            }
          } 
          
          check.remove(0);
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
              check.remove(0);
              mySerial.write("AT+ROLE0");  //coloca o modulo em modo slave para ficar disponivel a receber conexoes 
              check = getstr();
              return(0);
            }
          else if (check == "OK")
            {
              check.remove(0);
              mySerial.write("AT+ROLE0");  //coloca o modulo em modo slave para ficar disponivel a receber conexoes 
              check = getstr();
              return(-1);
            }
}

//------------------------------------------------------END DISCONNECT---------------------------------------------------------


