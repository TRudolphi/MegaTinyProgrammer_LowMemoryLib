#include <MegaTinyUtils.h>

MegaTinyUtils Utils;

#define TX_PIN PIN_PB2
 
void setup() 
{
  Utils.SerialBegin(TX_PIN,115200); // Init the serial port
  Utils.Delay(300);                 // Blocking start-delay
  Utils.Write("Starting\n");        // Serial write
  Utils.SleepInit(RTC_SECONDS_1);   // Base RTC time
}

void loop() 
{
  static int counter = 0;
  Utils.WriteNumber("counter = ",counter++,10,NEWLINE);
  Utils.Delay(5);                   // time the output the chars
  Utils.GoToSleep(4);               // Sleep for 4 times the base time
}
