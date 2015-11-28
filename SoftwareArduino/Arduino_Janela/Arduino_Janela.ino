/*
O comando AT disconecta o bluetooth case ele esteja conectado
Limpar o buffer serial antes de checar mensagem >> mySerial.flush()

*/

#include <SoftwareSerial.h>
#include "packet.h"

String master = "78A5048C378C";
String yellow = "78A5048C47AF";
String green = "78A5048C4A04";

//SoftwareSerial mySerial(11, 10); // RX, TX
const int botao1 = 8;     // the number of the pushbutton pin
const int botao2 = 7;
const int botao3 = 6; 
const int botao4 = 5;
int ROLE = 1;    //variavel que armazena o estado do bluetooth
String MAC = "";
String stin = "";
String stout = "";
int i = 0;
int conn;
char sum;
char cs;
int agua;
int estado = 0;

packet pkin, pkout;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
}

  pinMode(botao1, INPUT);
  pinMode(botao2, INPUT);
  pinMode(botao3, INPUT);
  pinMode(botao4, INPUT);
  
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

//------------------------------------------------------LOOP------------------------------------------------------------

void loop() // run over and over
{


//conecta
      if (digitalRead(botao1) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\nconectando\n");  //avisa que a rotina foi iniciada
          conn = connect(master);
          if (conn == 1) Serial.print("conectou\n");
          else
          { 
          if (conn == 0) Serial.print("erro\n");
          if (conn == -1) Serial.print("erro desconhecido\n");
          }
      }

//desconecta
      if (digitalRead(botao2) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\ndesconectando\n");  //avisa que a rotina foi iniciada
          conn = disconnect();
          if (conn == 0) Serial.write("desconectado");        
          else Serial.write("Ja estava desconectado");
      } 
     
//envia
      if (digitalRead(botao3) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
              Serial.write("\nenviando\n");
              send(yellow, "teste");
            //mySerial.write("1");
      }

//recebe
      if (digitalRead(botao4) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
              Serial.write("\nrecebendo\n");
              getpkt();
      }

//loop para passar as mensagens entre o serial virtual e o TX RX (serve para permitir que voce mande comandos ao modulo pelo terminal)    
/*      if (mySerial.available())
          Serial.write(mySerial.read());
      if (Serial.available())
          mySerial.write(Serial.read());
*/

//loop para esperar solicitacao do master
      if (mySerial.available())
      {
        pkin = getpkt();
        if (pkin.data == "OK+CONN")
          conn = 1;
        if (pkin.data == "OK+LOST")
          disconnect();
          conn = 0;
        if (pkin.data == "abrejanela")
        {
          digitalWrite(13,HIGH);
          estado = 1;
        } 
        if (pkin.data == "fechajanela")
        {
          digitalWrite(13,LOW);
          estado = 0;
        }
        if (pkin.data == "estadojanela")
        {
          delay(500);
          if (estado == 1) send (pkin.ids, "Janela aberta");
          if (estado == 0) send (pkin.ids, "Janela fechada");
        }
      }
}


      

//------------------------------------------------------END LOOP---------------------------------------------------------------
















