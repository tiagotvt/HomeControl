/*
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
String MAC = "";
String stin = "";
String stout = "";
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
/*      if (ROLE == 1 && conn != 1)
      {
          mySerial.write("AT+ROLE0");  //mantem o bluetooth em modo de slave, para que outros possam se conectar a ele
          delay(800);
          while (mySerial.available())
          Serial.write(mySerial.read());
          Serial.write("\n");
          ROLE = 0;
      }
*/
          MAC = "78A5048C47AF";
//conecta
      if (digitalRead(botao1) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\nconectando\n");  //avisa que a rotina foi iniciada
                  
          //bloco de conexao
          conn = connect(MAC);
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
              send(MAC, "teste");
      }

//recebe 
/*
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
*/
    
//loop para passar as mensagens entre o serial virtual e o TX RX (serve para permitir que voce mande comandos ao modulo pelo terminal)    
      if (mySerial.available())
          Serial.write(mySerial.read());
      if (Serial.available())
          mySerial.write(Serial.read());

}

//------------------------------------------------------END LOOP---------------------------------------------------------------
















