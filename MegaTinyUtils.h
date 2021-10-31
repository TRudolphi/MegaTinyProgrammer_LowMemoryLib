/*
  MegaTinyUtils.h - Library with low memory usage for:
  - serial port writing and reading
  - blockin and non blocking delay functions
  - sleep functionality 
  - Bit bang wire (I2C) routine
  - Read Vdd voltage (battery)
  
  Works with this MegaTiny library:
    https://github.com/SpenceKonde/megaTinyCore
  Created by Thomas Rudolphi, October, 2021.
  Released into the public domain.
*/
#ifndef MegaTinyUtils_h
#define MegaTinyUtils_h

#include "Arduino.h"
#include <avr/sleep.h>

// Serial
#define UROUND(x) ((2UL*(x)+1)/2)

#define NO_NEWLINE false
#define NEWLINE    true

// Sleep
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

// WireBitBang
extern const unsigned char SCL_PIN;
extern const unsigned char SDA_PIN;
#define I2C_DELAY 3 

#define READ_BIT  1

#define READ_NACK 0
#define READ_ACK  1

// don't use pinModeFast with INPUT_PULLUP! 

// ---- Macros
#define ClockHighNoDelay()  digitalWriteFast(SCL_PIN,1);
#define ClockLowNoDelay()   digitalWriteFast(SCL_PIN,0);
#define ClockLow()          digitalWriteFast(SCL_PIN,0); DelI2C();
#define ClockHigh()         digitalWriteFast(SCL_PIN,1); DelI2C();
#define WriteClockCycle()   DelI2C();digitalWriteFast(SCL_PIN,1);DelI2C();digitalWriteFast(SCL_PIN,0);
#define Start()             ClockHighNoDelay();pinMode(SDA_PIN,OUTPUT);digitalWriteFast(SDA_PIN,0); DelI2C(); ClockLowNoDelay() 
#define Stop()              pinMode(SDA_PIN,OUTPUT);digitalWriteFast(SDA_PIN,0); DelI2C(); ClockHigh(); digitalWriteFast(SDA_PIN,1);


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
    
    void SetupWireBb();
    void StartWireBb();
    void StopWireBb();
    bool WriteWire(unsigned char data);  
    unsigned char ReadWire(bool Ack);
    
    unsigned char ReadVdd(void);
  private:
    void PortLowPowerInit(void);
    unsigned long PrevMsDelay;
    unsigned long PrevMsDelayNb;
    void DelI2C(void);
};

#endif
