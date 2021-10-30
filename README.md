# MegaTinyUtils
   MegaTinyUtils.cpp - Arduino library with **low memory usage** for:
  - serial port writing and reading
  - blockin and non blocking delay functions
  - sleep functionality

  Works with this MegaTiny library:
    https://github.com/SpenceKonde/megaTinyCore
 
In most projects we want todo some logging via the serial port, but when using the Arduino Serial class,
this will takes a lot of memory (near 2KByte flash!). 
With this library this is less than 1kByte.

Functions:

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

### automatic switcher between UPDI and serial terminal circuit
![image](SerialSwitcher.jpg) "automatic switcher between UPDI and serial terminal"

This circuit automaticly switches between an UDPI firmware upload and serial monitoring in Arduino 
