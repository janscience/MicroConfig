/*
  FirmwareUpdate - Upload hex file from SD card
  Created by Jan Benda, August 15th, 2024.
*/

#ifndef FirmwareUpdate_h
#define FirmwareUpdate_h


#include <SD.h>


// List hex files on SD card.
int listFirmware(SDClass &sdcard, Stream &stream=Serial, bool number=false);

// Update firmware from hex file on SD card.
// Uses https://github.com/joepasquariello/FlasherX
void updateFirmware(SDClass &sdcard, bool echo=true, bool detailed=false,
		    Stream &instream=Serial, Stream &outstream=Serial);


#endif
