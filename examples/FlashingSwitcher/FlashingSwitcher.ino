/*
 * Firmware for a low/high voltage UPDI programmer with UART switching and reset functionality.
 * 
 * 3 types of programming possible:
 * JP1 placed:       Low voltage UDPI (factory default of a Attiny)
 * JP2 placed:       High voltage UDPI
 * JP1,2 not placed: programming when a bootloader is active in the target
 * JP3 with jumper:  the target is not reset (normal operation is without jumper!)
 * 
 * When the programming cycle is done, the RX/TX lines of the target are connected
 * to the PC, so the RS232 can be used for the debug traces.
 * When switching to the terminal or changing the baudrate, the target is also reset
 * (with a power cycle)
 * Now all is working just as with other arduino boards
 * 
 * For the realtime signals 2 internal events and the logic block are used:
 * Event2 
 *        out:      output to the target UPDI pin
 *        in:       PA1, TX of the PC
 *        function: out follows the input A1, or is set to HIGH (when tracing the target)
 *        
 * Event3 
 *        out:      output to the RX-pin of the PC
 *        in:       PB0, UPDI of target read 
 *             -Or- PB1, TX of target (when tracing the target)
 *        function: outpin follows the input PB0 Or PB1 
 *        
 * Logic block 
 *        out:      output to the target RX-pin
 *        in:       PA1, TX of the PC
 *        function: out follows the input A1 (when tracing the target), or is set to HIGH (while UPDI programming)
 *        
 *        Works best on the highest frequency (20MHz), for low interrupt latency
 *        
 * Author: T.Rudolphi
 *         The Netherlands
 *         december 2021
 *         https://github.com/TRudolphi/MegaTinyUtils
 *         
 */

#include <Event.h>
#include <Logic.h>

#define XMS               5       // Loop cycling of 5ms

// ---IO---
//#define OwnUpdiPin      PIN_PA0
#define PcTxPin           PIN_PA1 // logic block in and event2 in (PC-TX in)
#define TargetUpdiTxPin   PIN_PA2 // EVOUTA, event 2 out          (UPDI-out to target)
#define Prog12VPulsePin   PIN_PA3 // Output, low is 12V active
#define TargetRxPin       PIN_PA4 // logic block out              (To RX-target)
#define PowerOutPin       PIN_PA5 // For powering the target      (reset function)
#define ModePin           PIN_PA6 // Mode analog input
#define LED               PIN_PA7 // LED output 

#define TargetUpdiRxPin   PIN_PB0 // event3 in1                   (UPDI in)
#define TargetTxPin       PIN_PB1 // event3 in2                   (TX-target)
#define PcRxPin           PIN_PB2 // EVOUTB, event3 out           (UPDI or TX-target to PC-RX)
#define DtrPin            PIN_PB3 // input                        (DTRpin: LOW=RS232 terminal, HIGH=UPDI programming)
// ---IO---

#define UPDI_CHANNEL          0
#define UART_CHANNEL          1
#define NO_TARGET_CHANNEL     2

#define FLASH_VIA_UPDI        0
#define FLASH_VIA_UPDI_HV     1
#define FLASH_VIA_BOOTLOADER  2
#define MODE_UNDEFINED        3

#define RESET_TIME_MS 100    // ms
#define HV_TIME_US    200    // us

#define POWER_ON()    digitalWriteFast(PowerOutPin,LOW)  
#define POWER_OFF()   digitalWriteFast(PowerOutPin,HIGH) 

unsigned int PulsWidth;
bool CatchFirstBreak;
bool FallingEdge;
byte TargetChannel = NO_TARGET_CHANNEL;

ISR(PORTA_PORT_vect) 
{
  static unsigned int PulsStart;
  
  if(CatchFirstBreak)
  {  
    unsigned int PulsTime = micros(); // 6uS @20MHz
  
    if(digitalReadFast(PIN_PA1) == LOW)
    { // Falling edge
      PulsStart = PulsTime;
      FallingEdge = true;
    }
    else
    { // Rising edge   
      PulsWidth = PulsTime - PulsStart;
      if((PulsWidth > 65) && (PulsWidth < 200))
      { // Break at 115200 or 57600 baud detected
        CatchFirstBreak = false;
        digitalWriteFast(Prog12VPulsePin,LOW);
        delayMicroseconds(HV_TIME_US);   // 12Volt pulse for 200us
        digitalWriteFast(Prog12VPulsePin,HIGH);
        delayMicroseconds(5);         // float for 5uS (datasheet 1..10us) 
        digitalWriteFast(TargetUpdiTxPin,LOW);
        digitalWriteFast(PcRxPin,LOW);
        delayMicroseconds(85);        // Send handshake-break character
        digitalWriteFast(TargetUpdiTxPin,HIGH);
        digitalWriteFast(PcRxPin,HIGH);
        SetChannel(UPDI_CHANNEL);     // Hardware takes over, to follow the characters from the PC
      }
    }
  }
  else
  {
    if(digitalReadFast(PIN_PA1) == LOW)
    { // Falling edge
      FallingEdge = true;
    }     
  }
  PORTA.INTFLAGS = 0x02; // Reset PA1 interrupt
}
  
void SetChannel(byte Channel)
{
  if(Channel != NO_TARGET_CHANNEL)
  {
    if(Channel == UPDI_CHANNEL)
    { 
      Logic::stop();                              // Stop output of logic block     (TargetRxPin     HIGH level)                          
      Event2.set_user(user::evouta_pin_pa2);      // Set EVOUTA as event user       (TargetUpdiTxPin follow input)
      Event3.set_generator(gen3::pin_pb0);        // Set pin PB0 as event generator (TargetUpdiRxPin to PcRxPin)
    }
    else
    { // UART_CHANNEL
      Logic::start();                             // Start output PA4               (TargetRxPin     follow input) 
      Event2.clear_user(user::evouta_pin_pa2);    // Stop EVOUTA as event user      (TargetUpdiTxPin HIGH level) 
      Event3.set_generator(gen3::pin_pb1);        // Set pin PB1 as event generator (TargetTxPin     to PcRxPin)
    }
  }
  else
  { // No data to the Target
    Logic::stop();                                // Stop output PA4                (TargetRxPin     HIGH level)                          
    Event2.clear_user(user::evouta_pin_pa2);      // Stop EVOUTA as event user      (TargetUpdiTxPin HIGH level) 
    Event3.set_generator(gen3::pin_pb0);          // Set pin PB0 as event generator (TargetUpdiRxPin to PcRxPin)
  }
  Event3.set_user(user::evoutb_pin_pb2);          // Set EVOUTB as event user       (TX-Attiny to PC-RX)
  Event3.start(); 
  TargetChannel = Channel;   
}

void ActivatePA1Interrupt(bool activate)
{
  FallingEdge = false;
  if(activate == true)
  {
    SetChannel(NO_TARGET_CHANNEL);// No data from PC to the target
    PORTA.PIN1CTRL  = 0b00001001; // PULLUPEN=1, 1 = both edges, 2 = rising, 3 = falling
    CatchFirstBreak = true;    
  }
  else
  {
    PORTA.PIN1CTRL  = 0b00001000; // No interrupts
    CatchFirstBreak = false;      
  }
}

byte ReadMode(void)
{
  byte Mode; 
  int  AnalogValue = analogRead(ModePin); 
  if(AnalogValue < 100)
  { // 0 Volt
    Mode = FLASH_VIA_BOOTLOADER;
  }
  else if(AnalogValue < 768)
  { // 1.65 Volt
    Mode = FLASH_VIA_UPDI_HV;
  }
  else
  { // 3.3 Volt
    Mode = FLASH_VIA_UPDI;    
  }
  return Mode; 
}

void LedStatus(byte Mode)
{
  static uint16_t LedSequence_1      = 0b0000000000000001; // 1 flash per cycle
  static uint16_t LedSequence_2      = 0b0000000000000101; // 2 flashes per cycle 
  static uint16_t LedSequence_3      = 0b0000000000010101; // 3 flashes per cycle
  static uint16_t LedSequence_undef  = 0b0101010101010101; // flashes all the time 
  static uint16_t Mask = 1;

  Mask <<= 1;
  if (Mask >= 0x1000)
  {
    Mask = 0x0001;
  }

  switch (Mode)
  {
    case FLASH_VIA_UPDI:   
              digitalWriteFast(LED, (LedSequence_1 & Mask) > 0);
              break;
    case FLASH_VIA_UPDI_HV:   
              digitalWriteFast(LED, (LedSequence_2 & Mask) > 0);
              break;
    case FLASH_VIA_BOOTLOADER:   
              digitalWriteFast(LED, (LedSequence_3 & Mask) > 0);
              break;
    case MODE_UNDEFINED: 
    default:  digitalWriteFast(LED, (LedSequence_undef & Mask) > 0);
              break;    
  }
}

#define DTR_PIN_FALSE 0
#define DTR_PIN_TRUE  0x01
#define CHANGED       0x02
#define DTR_PIN_BIT   0x01

/**************************************************
 * Check the state of the DTRpin
 * 
 * return: 
 *        0x00 = No change,   DTRpin is false
 *        0x01 = No change,   DTRpin is true
 *        0x02 = pin changed, DTRpin is false
 *        0x03 = pin changed, DTRpin is true
 */
byte CheckDtrPin(bool DtrReRead)
{
  static bool DtrPinPrev = false;
  byte RetVal = 0;
  bool DTRpinValue = digitalReadFast(DtrPin);
  if(DtrReRead)
  {
    DtrPinPrev = DTRpinValue;
    RetVal     = CHANGED; // Always say changed
  }
  else
  {
    if(DTRpinValue != DtrPinPrev)
    {
      delay(2); // skip glitches
      DTRpinValue = digitalReadFast(DtrPin);
      if(DTRpinValue != DtrPinPrev)
      {
        DtrPinPrev = DTRpinValue;
        RetVal     = CHANGED;
      } 
    }
  }
  if(DTRpinValue == true)
  {
    RetVal |= DTR_PIN_TRUE;
  }
  return RetVal;   
}

void DoTargetReset(void)
{
  SetChannel(NO_TARGET_CHANNEL);
  digitalWriteFast(TargetRxPin,LOW);  // Set outputpin to 0 Volt, so it cannot supply the target
  POWER_OFF();
  delay(RESET_TIME_MS); 
  POWER_ON();
  digitalWriteFast(TargetRxPin,HIGH); // Set outputpin to high level 
  SetChannel(UART_CHANNEL);    
}

void setup() {
  delay(500);
  // ---EVENTS---
  Event2.set_generator(gen2::pin_pa1);          // Set pin PA1(PcTxPin) as event generator (PC-TX to RX-Attiny)
  Event3.set_user(user::evoutb_pin_pb2);        // Set EVOUTB as event user (TX-Attiny to PC-RX)

  SetChannel(NO_TARGET_CHANNEL);
    
  Event2.start();                               // Start the event channel2 once

  // ---LOGIC---                                // Initialize logic block 0
  Logic0.enable = true;                         // Enable logic block 0
  Logic0.input0 = in::unused;                   // don't use input PA0
  Logic0.input1 = in::input;                    // Set PA1 as input
  Logic0.input2 = in::unused;                   // don't use input PA2
  Logic0.output = out::enable;                  // Enable logic block 0 output pin (PA4)
  Logic0.truth = 0xCC;                          // Set truth table, just follow PA1
  Logic0.init();                                // Set the settings

  //Logic::start();                             // not yet start the output

  pinConfigure(ModePin, PIN_DIR_INPUT);
  pinConfigure(DtrPin,  PIN_PULLUP);
  pinConfigure(LED,     PIN_DIR_OUTPUT);
  
  digitalWriteFast(Prog12VPulsePin,HIGH);
  pinConfigure(Prog12VPulsePin,PIN_DIR_OUTPUT);
  
  pinConfigure(PowerOutPin,PIN_DIR_OUTPUT);
  POWER_ON();

  pinConfigure(TargetUpdiRxPin, PIN_PULLUP);
  pinConfigure(TargetTxPin, PIN_PULLUP);
        
  digitalWriteFast(TargetUpdiTxPin,HIGH);
  pinConfigure(TargetUpdiTxPin,PIN_DIR_OUTPUT); // Active drive pin high, also after reconnecting an event / logicblock
           
  digitalWriteFast(TargetRxPin,HIGH);
  pinConfigure(TargetRxPin,PIN_DIR_OUTPUT);     // Active drive pin high, also after reconnecting an event / logicblock
                 
  digitalWriteFast(PcRxPin,HIGH);
  pinConfigure(PcRxPin,PIN_DIR_OUTPUT);         // Active drive pin high, also after reconnecting an event / logicblock
 
  ActivatePA1Interrupt(false);
}

void loop()
{
  static byte Prescaler = 0;
  static byte ModePrev  = MODE_UNDEFINED;
  static unsigned long PrevEdgeMs = millis();
  
  byte DtrPinChange; 
  byte Mode = ReadMode();

  if(Mode != ModePrev)
  {
    delay(XMS); // skip glitches
    Mode = ReadMode();
    if(Mode != ModePrev)
    {
      ModePrev     = Mode;
      DtrPinChange = CheckDtrPin(true); // Refresh the DTR pin status
    }
  }
  else
  {
    DtrPinChange = CheckDtrPin(false);
  }
  
  switch(Mode)
  {
    case FLASH_VIA_UPDI_HV:     
      if((DtrPinChange & CHANGED) > 0)
      { // State-change of the DTR pin
        if((DtrPinChange & DTR_PIN_BIT) == DTR_PIN_TRUE)
        { // DTR is HIGH; UART for UDPI programming
          ActivatePA1Interrupt(true);
        }
        else
        { // DTR is LOW; UART to terminal
          ActivatePA1Interrupt(false);
          DoTargetReset();
        }        
      }
      else
      {
        if((DtrPinChange & DTR_PIN_BIT) == DTR_PIN_TRUE)
        { // UART for UDPI programming
          if(FallingEdge == true)
          {
            FallingEdge = false; 
            PrevEdgeMs  = millis();
          }
          if(CatchFirstBreak == false)
          {
            if(millis() - PrevEdgeMs > 5000)
            { // No activity for more than 5 seconds, set it 'sharp' for next programming cycle
              CatchFirstBreak = true;
              SetChannel(NO_TARGET_CHANNEL);
            }
          } 
        }
      }
      break;
                          
    case FLASH_VIA_BOOTLOADER:           
      if((DtrPinChange & CHANGED) > 0)
      { // State-change of the DTR pin
        ActivatePA1Interrupt(false);  
        if((DtrPinChange & DTR_PIN_BIT) == DTR_PIN_FALSE) // falling edge, reset (power-cycle) the target
        {
          DoTargetReset();
        }
        SetChannel(UART_CHANNEL);     
      } 
      break;
      
    case FLASH_VIA_UPDI:  
    default:                    
      if((DtrPinChange & CHANGED) > 0)
      { // State-change of the DTR pin
        ActivatePA1Interrupt(false);  
        
        if((DtrPinChange & DTR_PIN_BIT) == DTR_PIN_TRUE)
        { // UART for UDPI programming
          SetChannel(UPDI_CHANNEL);
        }
        else
        { // UART to terminal
          DoTargetReset();
        }        
      }
      else
      {
        if((DtrPinChange & DTR_PIN_BIT) == DTR_PIN_TRUE)
        { // UART for UDPI programming
          if(TargetChannel == UART_CHANNEL)
          {
            SetChannel(UPDI_CHANNEL);
            ActivatePA1Interrupt(false);  
          }
        }
        else
        {
          if(TargetChannel == UPDI_CHANNEL)
          {
            ActivatePA1Interrupt(false);
            DoTargetReset();                           
          }
        }                                  
      }
      break;
  }

  if(Prescaler > 0)
  {
    Prescaler--;
  }
  else
  {
    Prescaler = 175/XMS;
    LedStatus(Mode);
  }
  delay(XMS);
}
