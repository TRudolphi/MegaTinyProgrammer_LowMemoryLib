
/*
  MegaTinyUtils.cpp - Library with low memory usage for:
  - serial port writing and reading
  - blockin and non blocking delay functions
  - sleep functionality 
  Works with this MegaTiny library:
    https://github.com/SpenceKonde/megaTinyCore
  Created by Thomas Rudolphi, October, 2021.
  Released into the public domain.
*/
//#include "Arduino.h"
#include "MegaTinyUtils.h"

// Wake up routine
ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;        // Clear flag
}

MegaTinyUtils::MegaTinyUtils(void)
{
  PrevMsDelay = 0;
}

void MegaTinyUtils::Delay(unsigned int delayVal)
{
  PrevMsDelay = millis(); 
  while((millis() - PrevMsDelay) < delayVal); 
}

bool MegaTinyUtils::DelayNonBlock(unsigned int delayVal)
{
  if ( (millis() - PrevMsDelayNb) > delayVal )
  {
    PrevMsDelayNb = millis();
    return true;  
  }
  else
  {
    return false;
  }
}

void MegaTinyUtils::PortLowPowerInit(void)
{
  unsigned char i;
  for (i = 0; i < 8; i++)
  {
            *((unsigned char *)&PORTA + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
#ifdef PORTB
    if(i<6) *((unsigned char *)&PORTB + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
#endif
#ifdef PORTC
    if(i<4) *((unsigned char *)&PORTC + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;  
#endif
  }
}

void MegaTinyUtils::SleepInit(RTC_TIME_t RTC_time)
{
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;         // 1kHz Internal Crystal Oscillator (INT1K)
  while (RTC.STATUS > 0 || RTC.PITSTATUS);  // Wait for all register to be synchronized
  RTC.PITINTCTRL = RTC_PI_bm;               // Periodic Interrupt: enabled
  if(RTC_time > RTC_SECONDS_32) RTC_time = RTC_SECONDS_1;
  
  RTC.PITCTRLA = ((RTC_time + 1)<<3)
  | RTC_PITEN_bm;                           // Enable: enabled PIT

  PortLowPowerInit();
  sleep_enable();
}

void MegaTinyUtils::GoToSleep(unsigned char times)
{
  ADC0.CTRLA &= ~ADC_ENABLE_bm;     // disable ADC 
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_cpu ();                     
  for(unsigned char i=0;i<times-1;i++)
  {                                 
    sleep_cpu (); 
  }
  ADC0.CTRLA |= ADC_ENABLE_bm;      // enable ADC 
}  

void MegaTinyUtils::SerialBegin(unsigned char TxPin, long baudrate)
{
  pinMode(TxPin, OUTPUT);
#if F_CPU == 1000000  
  USART0.BAUD = UROUND(64UL*F_CPU/8/baudrate); // Must be >= 64
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm | USART_RXMODE0_bm; // Double-Speed (CLK2X)
#else
  USART0.BAUD = UROUND(64UL*F_CPU/16/baudrate);
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
#endif
  //USART0.CTRLC = (1 << USART_CHSIZE0_bp)|(1 << USART_CHSIZE1_bp); // 8 bit chars == default
}

bool MegaTinyUtils::Available(void)
{
  return (USART0.STATUS & USART_RXCIF_bm) != 0;
}

unsigned char MegaTinyUtils::Getchar(void)
{
  while(!Available());
  return USART0.RXDATAL; 
}

void MegaTinyUtils::Putchar(char data)
{
  while(!(USART0.STATUS & USART_DREIF_bm));
  USART0.TXDATAL = data; 
}

void MegaTinyUtils::Write(const char *rs232data)
{
  unsigned char i = 0;
  while(rs232data[i] != 0)
  {
    Putchar(rs232data[i++]);
  }
}

void MegaTinyUtils::WriteNumber(const char *rs232data, long int number, unsigned char base, bool NewLine)
{
  char number_buffer[7];

  ltoa (number,number_buffer,base);

  Write(rs232data);
  Write(number_buffer);
  if(NewLine) Putchar('\n');
}
