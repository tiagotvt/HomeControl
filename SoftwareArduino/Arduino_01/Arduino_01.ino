/*
Aplicar um delay de 800ms após um comando para garantir que o próximo será executado
O comando AT disconecta o bluetooth case ele esteja conectado
Limpar o buffer serial antes de checar mensagem >> mySerial.flsuh()


MASTER mac address: 78A5048C378C
YELLOW mac address: 78A5048C47AF
GREEN mac address:  78A5048C4A04

*/

#include <SoftwareSerial.h>
#include "packet.h"


//SoftwareSerial mySerial(11, 10); // RX, TX
const int botao1 = 9;     // the number of the pushbutton pin
const int botao3 = 8;
const int botao2 = 2; 
int ROLE = 1;    //variavel que armazena o estado do bluetooth
String check = "";
String MAC = "";
String stin = "";
String stout = "";
char c;
int i = 0;
int conn;
char sum;
char cs;

packet pkt, pkin, pkout;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
  // initialize the pushbutton pin as an input:
  pinMode(botao1, INPUT);
  pinMode(botao2, INPUT);
}

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

//------------------------------------------------------LOOP------------------------------------------------------------

void loop() // run over and over
{
  
//slave
      if (ROLE == 1 && conn != 1)
      {
          mySerial.write("AT+ROLE0");  //mantem o bluetooth em modo de slave, para que outros possam se conectar a ele
          delay(800);
          while (mySerial.available())
          Serial.write(mySerial.read());
          Serial.write("\n");
          ROLE = 0;
      }

          MAC = "78A5048C47AF";
//conecta
      if (digitalRead(botao1) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\nconectando\n");  //avisa que a rotina foi iniciada
                  
          //bloco de conexao
          conn = connect(MAC,0);
          if (conn == 1) Serial.print("conectou\n");
          else
          { 
          if (conn == 0) Serial.print("erro\n");
          if (conn == -1) Serial.print("erro desconhecido\n");
          }
          ROLE = 1;  //variavel de controle (o arduino nao sabe se o modulo esta como master ou slave)          

          }

//desconecta
      if (digitalRead(botao2) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\ndesconectando\n");  //avisa que a rotina foi iniciada
          //bloco de desconexão
          conn = disconnect();
          if (conn == 0) Serial.write("desconectado");        
          else Serial.write("Ja estava desconectado");
      } 
      
//envia
      if (digitalRead(botao3) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
              Serial.write("enviando\n");
              send(MAC,"cstart");
              pkin = getpkt();
              if (pkin.data == "okstart")
              {
                  send(MAC,"banana");
                  pkin = getpkt();
                  Serial.print(pkin.data);
              }
              else Serial.write("nao startou\n");
      }

//recebe 
      if (mySerial.available())    
      {
          stin = getstr();
          if (stin == "OK+CONN")
          {
              pkin = getpkt();
              if (pkin.data == "cstart")
              {
                  send(pkin.ids,"okstart");
                  pkin = getpkt();
                  cs = pkin.cs;
                  sum = checksum(pkin.data);
                  if (cs == sum)
                  {
                      Serial.write("checksum ok!\n");
                      send(pkin.ids,"checksum ok!"); 
                  }
                  else
                  { 
                      Serial.print(pkin.data);
                      Serial.write(" : ");
                      Serial.print(sum);
                      Serial.write("\n");
                      Serial.write("checksum falhou!\n");                 
                      send(pkin.ids,"checksum falhou");
                  }
              }
          }
      }
    
    
//loop para passar as mensagens entre o serial virtual e o TX RX (serve para permitir que voce mande comandos ao modulo pelo terminal)    
/*      if (mySerial.available())
          Serial.write(mySerial.read());
      if (Serial.available())
          mySerial.write(Serial.read());
*/
}

//------------------------------------------------------END LOOP------------------------------------------------------------

//------------------------------------------------------GETSTRING--------------------------------------------------------------
String getstr()
{
      String buff = "";
      int i = 0;
      
      while(!mySerial.available() && i<1000 ) //espera a informacao comecar a chegar. Se demorar mais de 5s cancela
      {
          i++;
          delay(5);
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

//------------------------------------------------------CONNECT-------------------------------------------------------------
int connect(String MAC, int err)
{
          if(err == 0)
          {
          mySerial.write("AT+ROLE1");  //coloca o modulo em modo master para se conectar a outro 
          delay(1500);
          mySerial.flush();
          }
          mySerial.print("AT+CON");  //conecta com o modulo especificado pelo mac
          mySerial.print(MAC);
          
          check = getstr();
         
          if (check == "OK+CONNA") //se verificado que o módulo entendeu o comando, limpa a string e prossegue
          {
          check.remove(0); //se o ble entendeu o comando, limpa a string e prossegue
          }
//          else 
//          {
//            if (check == "OK+CONNAOK+CONN") 
//            {
//              check.remove(0);
//              return(1); //para o caso da conexão ser estabelecida entre uma tentativa e outra
//            }
            else return (-1); //se nao, retornar erro desconhecido
//          }
          
          check = getstr();
          
          if (check == "OK+CONN")    //verifica se a conexao foi efetuada 
          {
          check.remove(0);
          return (1);
          }
          else    //caso a conexao tenha falhado, tenta novamente até 3 vezes
          {
              Serial.print("\nmilestone 2\n"); //debug
              err++;
              Serial.print(err); //debug
              check.remove(0);
              if (err >= 3) return (0);
              else return(connect(MAC, err));     
           }
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
              return(0);
            }
          else if (check == "OK")
            {
              check.remove(0);
              return(-1);
            }
}

//------------------------------------------------------END DISCONNECT---------------------------------------------------------


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


//------------------------------------------------------SEND--------------------------------------------------------------

int send(String idr, String data)
{
    //colocar o mac do modulo que esta sendo gravado aqui
    String ids = "78A5048C4A04";  //green
    //String ids = "78A5048C47AF";  //yellow
    
    String buff = "";
    buff = ids + idr + data;
    char cs = checksum(data);
    buff = buff + cs;
    Serial.write("foi : ");
    Serial.print(buff);
    Serial.write("\n");
    mySerial.print(buff);
}

//------------------------------------------------------SEND--------------------------------------------------------------







