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

#include "config.h"
#include "command.h"

#include <EEPROM.h>


static const uint32_t signature = 0xDEADBEEF;
static const int signature_eeprom_address = 0x00;      // uninitialized eeprom detection
static const int analog_port_eeprom_address = 0x10;    // 6 bytes 0x10-0x16
static const int digitial_port_eeprom_address = 0x20;  // 14 bytes 0x20-0x2e


void config_factory_reset (Config * board_config) 
{  
  board_config->signature = signature;
  board_config->debounce = 100;
  
  for (int i=0;i<ANALOG_PINS;i++ )
  {
    board_config->analog_port_config[i] = MODE_INPUT_PULLUP;   
  }
  for (int i=0;i<DIGITAL_PINS;i++ )
  {
    board_config->digital_port_config[i] = MODE_INPUT_PULLUP;    
  }
  
  config_write(board_config);  
}

void config_init(Config * board_config)
{
  EEPROM.get(0, *board_config);

  if ( board_config->signature != signature )
  {
    config_factory_reset(board_config);
  } 
}

void config_write(const Config * board_config) 
{
  EEPROM.put(0, *board_config);  
}


