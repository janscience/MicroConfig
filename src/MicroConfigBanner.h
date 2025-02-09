/*
  TeeRecBanner - ASCII art banner and MicroConfig version for output streams.
  Created by Jan Benda, February 9th, 2025.
*/

#ifndef MicroConfigBanner_h
#define MicroConfigBanner_h


#include <Arduino.h>


#define MICROCONFIG_SOFTWARE "MicroConfig version 0.3.0"


void printMicroConfigBanner(const char *software=NULL, Stream &stream=Serial);


#endif

