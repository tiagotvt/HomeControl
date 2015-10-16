/*
Aplicar um delay de 800ms após um comando para garantir que o próximo será executado
O comando AT disconecta o bluetooth case ele esteja conectado
Limpar o buffer serial antes de checar mensagem >> mySerial.flsuh()


MASTER mac address: 78A5048C378C
YELLOW mac address: 78A5048C47AF
GREEN mac address:  78A5048C4A04

*/

#include <SoftwareSerial.h>



SoftwareSerial mySerial(11, 10); // RX, TX
const int botao1 = 9;     // the number of the pushbutton pin
const int botao2 = 2; 
int ROLE = 1;    //variavel que armazena o estado do bluetooth
//char string check[20];  //string para armazenar mensagens de confirmacao
String check = "";
String MAC = "";
char c;
int i = 0;
int conn;
int dconn;

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
  if (ROLE == 1)
  {
  mySerial.write("AT+ROLE0");  //mantem o bluetooth em modo de slave, para que outros possam se conectar a ele
    delay(800);
    while (mySerial.available())
    Serial.write(mySerial.read());
    Serial.write("\n");
  ROLE = 0;
//  Serial.write("setou 0");
  }

      if (digitalRead(botao1) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\nconectando\n");  //avisa que a rotina foi iniciada
                  
          //bloco de conexao
          MAC = "78A5048C47AF";
          conn = connect(MAC,0);
          if (conn == 1) Serial.print("conectou\n");
          else
          { 
          Serial.print("conn = ");  
          Serial.print(conn);
          if (conn == 0) Serial.print("erro\n");
          if (conn == -1) Serial.print("erro desconhecido\n");
          }
          ROLE = 1;  //variavel de controle (o arduino nao sabe se o modulo esta como master ou slave)          

          }
          
      if (digitalRead(botao2) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\n\ndesconectando\n");  //avisa que a rotina foi iniciada
          //bloco de desconexão
          dconn = disconnect();
          if (dconn == 0) Serial.write("desconectado");        
          else Serial.write("Ja estava desconectado");
      } 
         
         //bloco de transmissao
         
//          mySerial.write("teste");
//          delay(800);
//          mySerial.write("AT");  //desconecta os modulos
//          delay(800);
//              while (mySerial.available())  //printa tudo que esta no bufffer
//              Serial.write(mySerial.read());
//              Serial.write("\n");
         

//      if (mySerial.available() == "teste")
//        Serial.write("entendi");
    
  //loop para passar as mensagens entre o serial virtual e o TX RX (serve para permitir que voce mande comandos ao modulo pelo terminal)    
  if (mySerial.available())
    Serial.write(mySerial.read());
  if (Serial.available())
    mySerial.write(Serial.read());

}

//------------------------------------------------------END LOOP------------------------------------------------------------


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
          delay(1000);
          while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string check --- intenção é receber OK+CONNA
          {
              c = mySerial.read();
              check += c;
              Serial.print(c); //debug
          }
          Serial.print("\n"); //debug
         
          if (check == "OK+CONNA") //se verificado que o módulo entendeu o comando, limpa a string e prossegue
          {
          check.remove(0); //se o ble entendeu o comando, limpa a string e prossegue
          Serial.print("milestone 1\n"); //debug
          }
          else 
          {
            if (check == "OK+CONNAOK+CONN") 
            {
              check.remove(0);
              return(1); //para o caso da conexão ser estabelecida entre uma tentativa e outra
            }
            else return (-1); //se nao, retornar erro desconhecido
          }
          
          while(!mySerial.available()) {} //espera a segunda parte da mensagem
          
          Serial.print("milestone 3\n"); //debug
          
          while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string check ---- a intenção é identificar o OK+CONN ou o OK+CONNF
          {
              c = mySerial.read();
              check += c;
              Serial.print(c); //debug
              delay(10);  //TIRAR ESSE DELAY QUEBRA O CODIGO DE JEITOS BIZARROS!!! 
          }
          Serial.print("\n"); //debug
          
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
          delay(1000);
          while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string check --- intenção é receber OK+LOST ou OK
          {
              c = mySerial.read();
              check += c;
          }
          Serial.println(check); //debug
          //Serial.print("\n"); //debug
          
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



