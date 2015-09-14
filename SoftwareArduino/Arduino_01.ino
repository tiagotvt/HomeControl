/*
Aplicar um delay de 800ms após um comando para garantir que o próximo será executado
O comando AT disconecta o bluetooth case ele esteja conectado
Limpar o buffer serial antes de checar mensagem >> mySerial.flsuh()


MASTER mac address: 78A5048C378C
YELLOW mac address: 78A5048C47AF

*/

#include <SoftwareSerial.h>



SoftwareSerial mySerial(11, 10); // RX, TX
const int botao = 9;     // the number of the pushbutton pin
int ROLE = 1;    //variavel que armazena o estado do bluetooth
//char string check[20];  //string para armazenar mensagens de confirmacao
String check = "";
String MAC = "";
char c;
int i = 0;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
  // initialize the pushbutton pin as an input:
  pinMode(botao, INPUT);
}

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

void loop() // run over and over
{
  if (ROLE == 1)
  {
  mySerial.write("AT+ROLE0");  //mantem o bluetooth em modo de slave, para que outros possam se conectar a ele
    delay(500);
    while (mySerial.available())
    Serial.write(mySerial.read());
    Serial.write("\n");
  ROLE = 0;
//  Serial.write("setou 0");
  }

      if (digitalRead(botao) == 1)  //verifica o estado do botao e executa se ele estiver pressionado
      {
          Serial.write("\nbotao\n");  //avisa que a rotina foi iniciada
          
          
          //bloco de conexao
          MAC = "78A5048C47AF";
          if (connect(MAC,0) == 1) Serial.print("conectou\n");
          else Serial.print("erro\n");
          ROLE = 1;  //variavel de controle (o arduino nao sabe se o modulo esta como master ou slave)          

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


int connect(String MAC, int err)
{
          mySerial.write("AT+ROLE1");  //coloca o modulo em modo master para se conectar a outro 
          delay(800);
          mySerial.flush();
          mySerial.print("AT+CON");  //conecta com o modulo especificado pelo mac
          mySerial.print(MAC);
          delay(800);
          while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string check
          {
              c = mySerial.read();
              check += c;
          }
          if (check == "OK+CONNAOK+CONN")    //verifica se a conexao foi efetuada 
          {
          check.remove(0);
          return (1);
          }
          else    //caso a conexao tenha falhado, tenta novamente ate 5 vezes
          {
              err++;
              check.remove(0);
              if (err >= 5) return (0);
              else connect (MAC, err);     
          }
}
