/*  Girder 6 IO Sketch
 *  ==================
 *  
 *  This code allows an Arduino compatible device to function as
 *  a cheap input output device for Girder 6.
 *    
 *    
 *  License
 *  =======
 *  Copyright 2016 (c) Promixis, LLC 
 *  Licensed for use with Promixis Girder.
 *  
 */
 
#include <Arduino.h>

#include "serial.h"
#include "command.h"

static char cb[7] = {0};
static uint8_t cbpos = 0;
static uint32_t serial_last_timestamp = 0;

#define CR  0x0d
#define LF 0x0a

static const char EndOfLine[2] = { CR, LF };

void serial_send( const char * cmd )
{  
  Serial.write(cmd,5); 
  Serial.write(EndOfLine,2); 
}

void serial_send( char command, char p1, char p2, char p3, char p4 )
{
  char buffer[5];
  buffer[0] = command;
  buffer[1] = p1;
  buffer[2] = p2;
  buffer[3] = p3;
  buffer[4] = p4;  
  serial_send(buffer);
}


void serial_send_error( uint8_t error )
{
  serial_send('E', '0','0', '0' + error / 10, '0' + error % 10 );  
}

void serial_process(uint32_t current_timestamp)
{   
  while ( Serial.available() > 0 ) 
  {    
    // reset input buffer if last character came in more than 5 seconds ago.
    if ( current_timestamp - serial_last_timestamp > 5000 )
    {
      cbpos = 0; 
    }  
    serial_last_timestamp = current_timestamp;
      
    char b = Serial.read();    
    
    switch ( b )
    {
      case 0x08: // backspace
        
        if ( cbpos > 0 ) // if another char delete it.
        {
          cbpos--;
        }
        break;
        
      case CR: // remove carriage return
      case LF: // line feed

        switch ( cbpos )
        {
          case 5:
            cb[cbpos] = 0;
            command_parse(current_timestamp, cb, cbpos);
            break;
          case 0:
            // just ignore empty line.
            break;
          default:                   
            serial_send_error(2);
        }
        
        cbpos = 0;
        break;        
        
      default:
        if ( cbpos < 5 )
        {
          cb[cbpos++] = toupper(b);
        }
        else
        {
          serial_send_error(2);
          cbpos = 0;
        }
    }
    
  } // while.
  
}

void serial_init()
{
  Serial.begin(9600);
}


