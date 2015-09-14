/*
 * serialRead.c:
 *	Example program to read bytes from the Serial line
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringSerial.h>

int main ()
{
  int fd ;

  if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

// Loop, getting and printing characters
  fflush(stdout);
//  delay(5000);
//  serialPuts (fd,"AT+MODE?");
//  serialPuts (fd,"AT+MODE?");
  serialPuts (fd,"AT");
  delay(500);



  int b = 0;
  int a = 0;

  for (;;)
  {
    serialPuts (fd,"AT+NAME?");
//    delay(500);
    while((a=serialDataAvail(fd))==0);
    delay(50);
    a = serialDataAvail(fd);
      for (a>=0;a--;){
    putchar (serialGetchar (fd)) ;
		}
    printf("%d\n",b);;
    b++;
    fflush (stdout) ;
    delay(2000);
  }
}
