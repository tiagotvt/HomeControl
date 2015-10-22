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





//------------------------------------------------------GETPACKET--------------------------------------------------------------

packet getpkt()
{
      char c;
      String buff = "";
      packet ret;
      int l;
      int i = 0;
      
      while(!mySerial.available() && i<1000 ) //espera a informacao comecar a chegar. Se demorar mais de 5s cancela
      {
          i++;
          delay(5);
      }

      if (i == 1000)
      {
          Serial.write("sem dados no serial\n");
          ret.ids = "000000000000";
          ret.idr = "000000000000";
          ret.data = "erro";
          ret.cs = 0;
          return(ret);
      }
    
      while (mySerial.available() > 0)  //transfere o buffer do mySerial para a string buff
      {
          c = mySerial.read();
          buff += c;
          delay(10);  //sem esse delay o while acaba antes de todos os dados passarem
      }
      Serial.write("chegou : ");
      Serial.print(buff); //debug
      Serial.print("\n"); //debug
      
      l = buff.length();
      ret.ids = buff.substring(0,12);
      ret.idr = buff.substring(12,24);
      ret.data = buff.substring(24,l-1);
      ret.cs = buff.charAt(l-1);
      
      return(ret);
}

//------------------------------------------------------END GETPACKET----------------------------------------------------------




