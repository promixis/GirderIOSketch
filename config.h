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
 
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

// how many analog pins do we have:
#define ANALOG_PINS 6
// how many digital pins do we have:
#define DIGITAL_PINS 14



typedef struct {
  uint32_t signature; // config internal use to determine if EEPROM was initialized or not.
  uint16_t debounce;
  uint8_t analog_port_config[ANALOG_PINS];
  uint8_t digital_port_config[DIGITAL_PINS];
} Config;


void config_factory_reset (Config * board_config);
void config_write(const Config * board_config);
void config_init(Config * board_config);


#endif

