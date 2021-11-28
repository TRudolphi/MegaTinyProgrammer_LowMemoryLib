# Circuit for (high voltage) programming an Attiny via the UDPI-pin, with serial tracing possibility 

New Attiny processors like the Attiny1606 / 414 etc. can be programmed via only one pin (reset pin).
On this page I show some circuits to do this, starting with the most basic one.

To have the same functionality as the standard Arduino's, after programming we like to switch back to the serial port
so it can be used for RS232 tracing and debugging. This will be part of the designs.

All is based on the pinning of a very standard FTDI-USB-RS232 serial interface

## basic UPDI only programmer

![image](UpdiOnly.jpg)

Pro: very simple design<br>
Con: No tracing possible via the same serial interface

## UPDI and UART switcher

![image](UpdiAndSwitcher.jpg)

Pro:
* simple design<br>
* when Arduino terminal is active, Attiny uart pins are switched to the PC RS232

Con:<br>
* No reset of the target possible (without programming)<br>
* When a bootloader is active in the target, this can not be used (no reset)<br>

## UPDI and UART switcher with reset function

![image](UpdiProgrammerSwitcherWithReset.jpg)

Pro:
* when Arduino terminal is active, Attiny uart pins are switched to the PC RS232

Con:<br>
* No high voltage UPDI possible<br>

With the jumper JP1 two ways of controller flashing can be choosen:

### 1-2 => UPDI programming / serial terminal

In this mode the circuit automaticly switches between an UDPI sketch upload and serial monitoring in Arduino

### 2-3 => Flashing with an internal bootloader / serial terminal
When a bootloader is present in the controller (via Burn Bootloader), you need to reset the controller
before flashing a new sketch in it. We want to keep the UPDI program functionality on the updi-pin,
and also want to keep all I/O-pins for other purposes (other than having a software-reset-function).

To reset the controller, it is also possible to remove the supply voltage for a short while.
The DTR pin is going just before programming from VDD to 0 Volt, via C3 and filter R3-C2 the switch Z is set
to Z0 for a short while (200ms). During this time Q1 is not conducting, so the VDD for the target is removed.
R5 is present to have a low resistant pull-down path, so the parasitic supply by the TX pin is not enough
to keep the part out of reset.

### After the flashing
After the flash action, the serial monitor within Arduino becomes active again (if started via the menu),
so it all works just as with an ordinary arduino board.

