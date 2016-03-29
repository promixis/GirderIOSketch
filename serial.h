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
 
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

void serial_send( const char * cmd );
void serial_send( char command, char p1, char p2, char p3, char p4 );
void serial_send_error( uint8_t error );
void serial_process(uint32_t current_timestamp);
void serial_init();

#endif
