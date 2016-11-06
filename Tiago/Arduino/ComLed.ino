/*
O comando AT disconecta o bluetooth case ele esteja conectado
Limpar o buffer serial antes de checar mensagem >> mySerial.flush()

*/
#include <PWMServo.h>
#include <SoftwareSerial.h>
#include "packet.h"

//SERVO
int pos = 7;
int servo = 9;
PWMServo myservo;  // create servo object to control a servo

int inPin = 12;   // choose the input pin (for a pushbutton)
int val = 0;     // variable for reading the pin status
int flag = 0;

//LEDS
int redPin = 11;
int greenPin = 10;
int bluePin = 6;
//MAC ADDRESS
String master = "A4D57812B725";
String yellow = "78A5048C47AF";
String green = "A4D57869D3D0";
//Sensor Umidade
const int pin = A0;
int umidade = 0;
int angulo = 0;
//SoftwareSerial mySerial(11, 10); // RX, TX
const int botao1 = 8;     // the number of the pushbutton pin
const int botao2 = 7;
const int botao3 = 6; 
const int botao4 = 5;
int ROLE = 1;    //variavel que armazena o estado do bluetooth
String MAC = master;
String stin = "";
String stout = "";
int i = 0;
int conn;
char sum;
char cs;
int agua;
int trigger = 0;

packet pkin, pkout;

void setup()
{
  //SERVO
  myservo.attach(9);
  pinMode(13, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(inPin, INPUT);
  digitalWrite(13, HIGH);
  myservo.write(pos);
  //SET THE LEDs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
}

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  mySerial.print("AT+ROLE0");
  delay(800);
  //MAC = "AT+CON" + MAC;
  mySerial.print("AT+CONA4D57812B725");
}

//------------------------------------------------------LOOP------------------------------------------------------------

void loop() // run over and over
{
      val = digitalRead(inPin);  // read input value
      if (val == HIGH) {         // check if the input is HIGH (button released)
                if(flag == 1)
                {
                flag = 0;
                setColor(0, 0, 0);
                Serial.println("DESLIGANDO...");
                delay(10000);
                }
      }
        while(flag == 0)
        {
              val = digitalRead(inPin);  // read input value
              if (val == HIGH) {         // check if the input is HIGH (button released)
                Serial.println("LIGADO!");
                flag = 1;
                setColor(128, 255, 0);
                delay(10000);
              } else {
                Serial.println("OFF!");
                setColor(255, 0, 0);
                delay(1000);
              }
        }
/*
//conecta
      if (botao1 == 1)  //verifica o estado do botao e executa se ele estiver pressionado
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
      if (botao2 == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\ndesconectando\n");  //avisa que a rotina foi iniciada
          conn = disconnect();
          if (conn == 0) Serial.write("desconectado");        
          else Serial.write("Ja estava desconectado");
      } 
     
//envia
      if (botao3 == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
              Serial.write("\nenviando\n");
              send(yellow, "teste");
            //mySerial.write("1");
      }

//recebe
      if (botao4 == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
              Serial.write("\nrecebendo\n");
              getpkt();
      }
*/
//loop para passar as mensagens entre o serial virtual e o TX RX (serve para permitir que voce mande comandos ao modulo pelo terminal)    
/*      if (mySerial.available())
          Serial.write(mySerial.read());
      if (Serial.available())
          mySerial.write(Serial.read());
*/

//leitura do sensor de agua

      mySerial.flush();
      Serial.flush();
      delay(100);
//loop para esperar solicitacao do master
            if (mySerial.available())
            {
              
              pkin.data = getstr();
              delay(100);
              
              if (pkin.data == "OK+CONN")
                conn = 1;
              if (pkin.data == "OK+LOST")
                disconnect();
                conn = 0;
              if (pkin.data.substring(0, 4) == "agua")
              {
                delay(800);
                angulo = pkin.data.substring(4,7).toInt();
                Serial.println(pkin.data.substring(4,7));
                //mySerial.write("AT+ROLE1");  //coloca o modulo em modo master para se conectar a outro
                delay(800);
                for(i = 0;i<10;i++){
                  delay(100);
                  umidade += analogRead(pin);
                }
                umidade = umidade/10;
                umidade = map(umidade, 476, 1023, 100, 0);
                //Serial.print("Umidade media mapeada = ");
                Serial.println(umidade);
                delay(100);
                
                //mySerial.print("AT+CONA4D57812B725");
                delay(100);
                //mySerial.write(umidade);
                stout = umidade;
                
                delay(500);
                if(umidade < 30){
                  pos=angulo;
                  //pos = map(angulo, 0, 180, 7, 170);
                  myservo.write(pos);
                  Serial.print("Umidade de: ");
                  Serial.println(umidade);
                  Serial.println("Preparando para irrigacao");
                  Serial.print("Sprinkler com angulo: ");
                  Serial.println(angulo);
                  delay(2000);
                  Serial.println("INICIO DA IRRIGACAO");
                  digitalWrite(3,HIGH);
                  setColor(80, 0, 80); // purple
                  Serial.print("[");
                  for(i=0; i < 10; i++)
                  {
                    Serial.print("..");
                    delay(1000);
                  }
                  Serial.print("]\n");
                  Serial.println("FIM DA IRRIGACAO");
                  digitalWrite(3,LOW);
                }
                if(umidade > 30){
                  Serial.print("Umidade de: ");
                  Serial.println(umidade);
                  Serial.println("NAO PRECISA SER MOLHADA");
                  setColor(0, 255, 0); // green
                  delay(10000);
                }
                //mySerial.write("AT+ROLE0");
                //send(master, stout);
                umidade = 0;
              }
              Serial.println("ON");
              setColor(128, 255, 0);
            }
            delay(1000);

}

//------------------------------------------------------END LOOP---------------------------------------------------------------
void setColor(int red, int green, int blue)
{
#ifdef COMMON_ANODE
red = 255 - red;
green = 255 - green;
blue = 255 - blue;
#endif
analogWrite(redPin, red);
analogWrite(greenPin, green);
analogWrite(bluePin, blue);
}















