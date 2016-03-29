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

#include "command.h"
#include "serial.h"

static Config * board_config;

typedef struct {
  uint8_t state;
  uint16_t debounce;  
} Debounce;

static Debounce analog_in_state[ANALOG_PINS] = {0};
static Debounce digital_in_state[DIGITAL_PINS] = {0};

static bool command_pin_mode_is_input( int mode )
{

  switch ( mode )
  {
    case MODE_INPUT_INVERT:
    case MODE_INPUT:
      return true;      
    case MODE_INPUT_PULLUP_INVERT:
    case MODE_INPUT_PULLUP:
      return true;
      break;
    case MODE_OUTPUT_INVERT:
    case MODE_OUTPUT:
      return false;
    default:
      return true;
  }    
}

static bool command_pin_mode_inverted( int mode )
{
  return ( mode & 0x04 ) == 0x04 ;  
}


static int command_pin_mode_to_arduino_pin_mode( int mode )
{
  switch ( mode )
  {
    case MODE_INPUT_INVERT:
    case MODE_INPUT:
      return INPUT;      
    case MODE_INPUT_PULLUP_INVERT:
    case MODE_INPUT_PULLUP:
      return INPUT_PULLUP;
      break;
    case MODE_OUTPUT_INVERT:
    case MODE_OUTPUT:
      return OUTPUT;
    default:      
      return -1;        
  }  
}

static uint16_t getval(const char * command, uint8_t pos, uint8_t digits)
{
  uint16_t v = 0;
  if ( pos + digits > 5 )
  {
    return 0xffff;
  }
  
  for (int i=0;i<digits;i++)
  {
    v = v * 10;
    char digit = command[pos+i];
    v += uint8_t(digit - '0');
  }
  return v;
}

static uint16_t pow10(uint8_t exp)
{
    uint16_t v = 1;
    for (uint8_t i = 0; i < exp; i++)
    {
        v = v * 10;
    }
    return v;
}

static char * setval(char * command, uint16_t value, uint8_t pos, uint8_t digits)
{
    if (pos + digits > 5)
    {
        return command;
    }

    for (int i = 0; i<digits; i++)
    {
        int divisor = pow10(digits - i - 1);
        int d = value / divisor;
        value = value % divisor;

        if (d > 10)
        {
            d = d % 10;
        }

        command[pos + i] = '0' + d;
    }

    return command;

}

static void command_init_state()
{
  for (int i=0;i<ANALOG_PINS;i++)
  {
    if ( command_pin_mode_is_input(board_config->analog_port_config[i]) )
    {
      analog_in_state[i].state = digitalRead(A0 + i);
      analog_in_state[i].debounce = 0;      
    } 
  }

  for (int i=2;i<DIGITAL_PINS;i++)
  {
    if ( command_pin_mode_is_input(board_config->digital_port_config[i]) )    
    {
      digital_in_state[i].state = digitalRead(i);
      digital_in_state[i].debounce = 0;
    } 
  }    
}


void command_check_state(uint32_t current_timestamp)
{

  for (int i=0;i<ANALOG_PINS;i++)
  {
    int state = digitalRead(A0 + i);
    if ( command_pin_mode_is_input(board_config->analog_port_config[i]) && ( state != analog_in_state[i].state ))
    {
      analog_in_state[i].debounce++;
      if ( analog_in_state[i].debounce > board_config->debounce )
      {
      
        analog_in_state[i].state = state;
        analog_in_state[i].debounce = 0;
        
        char buf[5];
        buf[0]='N';
        buf[1]='A';
        setval(buf, i, 2,2);
        if ( command_pin_mode_inverted(board_config->analog_port_config[i]) )
        {
          buf[4]='0' + ( state == HIGH ? 0 : 1 );
        }
        else
        {
          buf[4]='0' + ( state == HIGH ? 1 : 0 );
        }
        serial_send(buf);
      }
    } 
    else
    {
      analog_in_state[i].debounce = 0;
    }
  }

  for (int i=2;i<DIGITAL_PINS;i++)
  {
    int state = digitalRead(i);
    if ( command_pin_mode_is_input(board_config->digital_port_config[i]) && ( state != digital_in_state[i].state ))
    {
      digital_in_state[i].debounce++;
      if ( digital_in_state[i].debounce > board_config->debounce ) 
      {
        digital_in_state[i].state = state;
        digital_in_state[i].debounce = 0;
        char buf[5];
        buf[0]='N';
        buf[1]='D';
        setval(buf, i, 2,2);
        if ( command_pin_mode_inverted(board_config->digital_port_config[i]) )
        {
          buf[4]='0' + ( state == HIGH ? 0 : 1 );
        }
        else
        {
          buf[4]='0' + ( state == HIGH ? 1 : 0 );
        }
        serial_send(buf);
      }
    } 
    else
    {
      digital_in_state[i].debounce = 0;
    }
  }  
    
}



static void command_set_ddr(const char * command)
{
  if (!( command[1] == 'D' || command[1] == 'A' ))
  {
    serial_send_error(5);
    return;
  }


  uint16_t val = getval(command,4,1);
  
  if ( val > 6 || val == 3 )
  {
    serial_send_error(4);
    return;
  }

  int pin = getval(command,2,2);
  
  if ( command[1] == 'D' )
  {       
    if ( pin < 2 || pin >= DIGITAL_PINS )
    {
      serial_send_error(3);
      return; 
    }
    board_config->digital_port_config[pin] = val;
  }
  
  if ( command[1] == 'A' )
  {    
    if ( pin >= ANALOG_PINS )
    {
      serial_send_error(3);
      return; 
    }
    board_config->analog_port_config[pin] = val;
    pin += A0;
  }
  
  pinMode(pin, command_pin_mode_to_arduino_pin_mode(val));
  config_write(board_config);
  
  serial_send(command);

  command_init_state();
  
}

static void command_get_ddr(const char * command)
{
  if (!( command[1] == 'D' || command[1] == 'A' ))
  {
    serial_send_error(5);
    return;
  }

  int pin = getval(command,2,2);
  int mode = 0;
    
  if ( command[1] == 'D' )
  {       
    if ( pin < 2 || pin >= DIGITAL_PINS )
    {
      serial_send_error(3);
      return; 
    }
    mode = board_config->digital_port_config[pin];
  }
  
  if ( command[1] == 'A' )
  {    
    if ( pin >= ANALOG_PINS )
    {
      serial_send_error(3);
      return; 
    }
    mode = board_config->analog_port_config[pin];
    pin += A0;
  }


  char buf[5];
  strcpy(buf, command);  
  setval(buf, mode, 4,1);
  serial_send(buf);  
}

static void command_ddr(const char * command)
{
  if ( command[4] == '?' )
  {
    command_get_ddr(command);
  }
  else
  {
    command_set_ddr(command);
  }
}

static void command_debounce(const char * command)
{
  if ( command[1] == '?' ) 
  {
    char buf[5];
    strcpy(buf, command);  
    setval(buf, board_config->debounce, 1,4);
    serial_send(buf);
  }
  else
  {
    board_config->debounce = getval(command, 1, 4);    
    serial_send(command);
  }  
}

static void command_get_pin(const char * command)
{
  if (!( command[1] == 'D' || command[1] == 'A' ))
  {
    serial_send_error(5);
    return;
  }

  int pin = getval(command,2,2);
  int mode = 0;
  if ( command[1] == 'D' )
  {       
    if ( pin < 2 || pin >= DIGITAL_PINS )
    {
      serial_send_error(3);
      return; 
    }
    mode = board_config->digital_port_config[pin];
    
  }
  
  if ( command[1] == 'A' )
  {    
    if ( pin >= ANALOG_PINS )
    {
      serial_send_error(3);
      return; 
    }
    mode = board_config->analog_port_config[pin];
    pin += A0;
  }
 
  int val = digitalRead(pin);

  char buf[5];
  strcpy(buf, command);  
  if (command_pin_mode_inverted(mode))
  {
    buf[4] = val == HIGH ? '0' : '1';
  }
  else
  {
    buf[4] = val == HIGH ? '1' : '0';
  }
  serial_send(buf);
}

static void command_set_pin(const char * command)
{
  if (!( command[1] == 'D' || command[1] == 'A' ))
  {
    serial_send_error(5);
    return;
  }

  int pin = getval(command,2,2);
  int mode;
  if ( command[1] == 'D' )
  {       
    if ( pin >= DIGITAL_PINS )
    {
      serial_send_error(3);
      return; 
    }
    mode = board_config->digital_port_config[pin];
  }
  
  if ( command[1] == 'A' )
  {    
    if ( pin >= ANALOG_PINS )
    {
      serial_send_error(3);
      return; 
    }
    mode = board_config->analog_port_config[pin];
    pin += A0;
  }

  uint16_t val = getval(command,4,1); 
  if (command_pin_mode_inverted(mode))
  {
    digitalWrite(pin, val != 0 ? LOW: HIGH);
  }
  else
  {
    digitalWrite(pin, val != 0 ? HIGH : LOW);
  }
  
  serial_send(command);
}

static void command_pin(const char * command)
{
  if ( command[4] == '?' )
  {
    command_get_pin(command);
  }
  else
  {
    command_set_pin(command);
  }
}

static void command_factory_reset(const char * command )
{
  config_factory_reset(board_config);
  command_init(board_config);
  serial_send(command);  
}

void command_parse(uint32_t current_timestamp, const char * command, int len)
{
  if ( len != 5 )
  {
    serial_send_error(3);
    return;
  }
  
  switch (command[0])
  {
  case 'V':
    serial_send("V0100"); 
    break;
  case 'D':
    command_ddr(command);
    break;
  case 'B':
    command_debounce(command);
    break;
  case 'P':
    command_pin(command);    
    break;    
  case 'R':
    command_factory_reset(command);
    break;  
    
  default:
    serial_send_error(4);
    
  }  
}


void command_init(Config * _board_config)
{
  board_config = _board_config;
  
  for (int i=0;i<ANALOG_PINS;i++ )
  {    
    pinMode(A0+i, command_pin_mode_to_arduino_pin_mode(board_config->analog_port_config[i]));
  }
  for (int i=2;i<DIGITAL_PINS;i++ )
  {   
    pinMode(i, command_pin_mode_to_arduino_pin_mode(board_config->digital_port_config[i]));
  }  

  command_init_state();
    
}








