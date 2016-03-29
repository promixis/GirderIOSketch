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
 
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdint.h>

#include "config.h"

#define MODE_INPUT                0
#define MODE_INPUT_PULLUP         1
#define MODE_OUTPUT               2

#define MODE_INPUT_INVERT         4
#define MODE_INPUT_PULLUP_INVERT  5
#define MODE_OUTPUT_INVERT        6

void command_parse(uint32_t current_timestamp, const char * command, int len);
void command_init(Config * _board_config);
void command_check_state(uint32_t current_timestamp);

#endif

