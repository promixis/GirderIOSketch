Girder 6 IO Sketch
==================
This code allows an Arduino compatible device to function as
a cheap input output device for Girder 6.

License
-------
Copyright 2016 (c) Promixis, LLC 
Licensed for use with Promixis Girder.

Usage
-----
All but the serial ports can be configured as input/outputs by using the
serial protocol. Events are generated when a level changes.

Protocol
--------

The protocol is a fixed length, CR terminated ASCI protocol for easy parsing
easy manual control if needed.

CXXXX[CR]

* C    = Command  [A-Z]
* XXXX = Parameters [0-9, A-Z]

Protocol Commands
-----------------
The Arduino Uno has 6 analog pins (A, 0-5) and 14 digital pins, however the 
first two digital pins 0 and 1 are used by the serial communications so 2-13 '
are available.

Baud rate is 9600, 8 bits, 1 stop bit.

### ERROR RESPONSE

If any command was not understood the response is
Response: EXXXX[CR] XXXX error code.

Error codes:

* 0001 protocol error
* 0002 unknown command
* 0003 invalid pin number
* 0004 invalid pin mode
* 0005 invalid must choose A or D for analog or digital pin.

### GETVERSION

Request: V0000[CR] 

Response: VHHLL[CR]  answers with version of firmware.

### DDR

Set direction and polarity.

Request: DXYYZ[CR] X = [A (analog pins), D(digital pins)]. YY is pin number. Z is the polarity and mode or ? to request current mode.

Response: DXYYZ[CR] echos request or error code 0002.

Available modes

* 0 = INPUT
* 1 = INPUT WITH PULLUP
* 2 = OUTPUT
* 4 = INPUT INVERTED
* 5 = INPUT WITH PULLUP INVERTED
* 6 = OUTPUT INVERTED

### DEBOUNCE

Request: BXXXX[CR] X = debounce value, if X is all ? it returns the current debounce value.

Answer: BXXXX[CR]

### PIN

Request: PXYYZ[CR] X = [A (analog pins), D(digital pins)]. YY is pin number. Z is 0, 1 or ?. ? requests current state without changing.

Response: PXYYZ[CR], echos first part adds Z which is 1 if pin is high, zero if pin is low.

### PIN NOTIFICATION

This is sent without a request if a pin level changes.
Notification: NXYYZ, X = [A,D], Y = pin number, Z = level

### RESET

Reset configuration to factory defaults.
R0000[CR]
