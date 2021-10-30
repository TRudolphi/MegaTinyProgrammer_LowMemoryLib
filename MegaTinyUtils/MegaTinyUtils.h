/*
  MegaTinyUtils.h - Library with low memory usage for:
  - serial port writing and reading
  - blockin and non blocking delay functions
  - sleep functionality 
  Works with this MegaTiny library:
    https://github.com/SpenceKonde/megaTinyCore
  Created by Thomas Rudolphi, October, 2021.
  Released into the public domain.
*/
#ifndef MegaTinyUtils_h
#define MegaTinyUtils_h

#include "Arduino.h"
#include <avr/sleep.h>

#define UROUND(x) ((2UL*(x)+1)/2)

#define NO_NEWLINE false
#define NEWLINE    true

typedef enum RTC_TIME_enum
{
    RTC_MSEC_4     = 0,
    RTC_MSEC_8     = 1,
    RTC_MSEC_16    = 2,
    RTC_MSEC_31    = 3,
    RTC_MSEC_62    = 4,
    RTC_MSEC_125   = 5,
    RTC_MSEC_250   = 6,
    RTC_MSEC_500   = 7, // 1024/512=2Hz,500ms
    RTC_SECONDS_1  = 8, // 1024/1024=1Hz,1sec
    RTC_SECONDS_2  = 9, // 1024/2048=1/2Hz,2sec
    RTC_SECONDS_4  = 10,
    RTC_SECONDS_8  = 11,
    RTC_SECONDS_16 = 12,
    RTC_SECONDS_32 = 13
} RTC_TIME_t;

class MegaTinyUtils
{
  public:
    MegaTinyUtils(void);
    void Delay(unsigned int delayVal);
    bool DelayNonBlock(unsigned int delayVal);
    
    void SleepInit(RTC_TIME_t RTC_time);
    void GoToSleep(unsigned char times);
    
    void SerialBegin(unsigned char TxPin, long baudrate);
    bool Available(void);
    unsigned char Getchar(void);
    void Putchar(char data);
    void Write(const char *rs232data);
    void WriteNumber(const char *rs232data, long int number, unsigned char base, bool NewLine);
  private:
    void PortLowPowerInit(void);
    unsigned long PrevMsDelay;
    unsigned long PrevMsDelayNb;
};

#endif
