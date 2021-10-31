#include <MegaTinyUtils.h>

MegaTinyUtils Utils;

#define TX_PIN PIN_PB2

void setup() 
{
  Utils.SerialBegin(TX_PIN,115200); // Init the serial port
  Utils.Delay(300);                 // Blocking start-delay
  byte Vdd = Utils.ReadVdd();       // Read the Vdd of the part

  Utils.WriteNumber("Vdd = ",Vdd/10,10,NO_NEWLINE);
  Utils.WriteNumber(".",Vdd%10,10,NO_NEWLINE);  
  Utils.Write(" Volt\n");
}

void loop() 
{
}
