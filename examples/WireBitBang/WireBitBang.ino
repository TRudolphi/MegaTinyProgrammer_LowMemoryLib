#include <MegaTinyUtils.h>

MegaTinyUtils Utils;

#define TX_PIN PIN_PB2

#define WIRE_ADDRESS (0x1E << 1)

const unsigned char SCL_PIN = PIN_PB0; // Pins to be used for the wire communication
const unsigned char SDA_PIN = PIN_PB1;

uint8_t _buffer[8];

void setup() 
{
  Utils.SerialBegin(TX_PIN,115200); // Init the serial port
  Utils.Delay(300);                 // Blocking start-delay
  Utils.Write("Starting\n");        // Serial write
  Utils.SetupWireBb();              // Init the wire routine
}

void loop() 
{
  WireWrite(0,0x55);
  uint8_t* buffer = WireRead(0x09, 1);
  Utils.WriteNumber("buffer[0]:",buffer[0],10,NEWLINE);
  Utils.Delay(1000);
}

void WireWrite(short address, short data)
{
  Utils.StartWireBb();
  if(Utils.WriteWire(WIRE_ADDRESS) == true)
  {
    Utils.WriteWire(address);
    Utils.WriteWire(data);
    Utils.Write("ACK\n");
  }
  else
  {
    Utils.Write("NACK!\n");
  }
  Utils.StopWireBb();  
}

uint8_t* WireRead(short address, short length) 
{
  byte i;
  Utils.StartWireBb();
  Utils.WriteWire(WIRE_ADDRESS);
  Utils.WriteWire(address);

  Utils.StartWireBb(); // Restart
  Utils.WriteWire(WIRE_ADDRESS + READ_BIT);
  for (i = 0; i < (length - 1) && i < (sizeof(_buffer) - 1); i++) {
    _buffer[i] = Utils.ReadWire(READ_ACK);
  }
  _buffer[i] = Utils.ReadWire(READ_NACK); // Last byte give a NACK
  Utils.StopWireBb();
  return _buffer;
}
