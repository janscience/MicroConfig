#include <Arduino.h>
#include <Action.h>
#include <FirmwareUpdate.h>

// From https://github.com/joepasquariello/FlasherX :
#include "FlasherX/FXUtil.h"     // read_ascii_line(), hex file support
extern "C" {
  #include "FlasherX/FlashTxx.h" // TLC/T3x/T4x/TMM flash primitives
}


int listFirmware(SDClass &sdcard, Stream &stream, bool number) {
  SdFile file;
  SdFile dir;
  if (!dir.open("/")) {
    stream.printf("! ERROR: Faild to access root folder on SD card.\n");
    return 0;
  }
  stream.println("Available firmware files on SD card:");
  int n = 0;
  while (file.openNext(&dir, O_RDONLY)) {
    if (!file.isDir()) {
      char fname[200];
      file.getName(fname, 200);
      if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".hex") == 0) {
	if (number)
	  stream.printf("- %d) ", n+1);
	else
	  stream.print("  ");
	stream.println(fname);
	n++;
      }
    }
  }
  if (n == 0)
    stream.printf("No firmware files found.\n");
  return n;
}


void updateFirmware(SDClass &sdcard, bool echo, bool detailed,
		    Stream &instream, Stream &outstream) {
  // list firmware files:
  int n = listFirmware(sdcard, outstream, true);
  outstream.println();
  if (n == 0)
    return;
  // select firmware file:
  int m = 0;
  if (detailed || n > 1) {
    while (true) {
      outstream.print("Select a firmware file [1]: ");
      while (instream.available() == 0) {
	yield();
	delay(1);
      }
      char pval[32];
      instream.readBytesUntil('\n', pval, 32);
      if (strlen(pval) == 0)
	strcpy(pval, "1");
      outstream.println(pval);
      char *end;
      long i = strtol(pval, &end, 10) - 1;
      if (end != pval && i >= 0 && i < (long)n) {
	m = i;
	outstream.println();
	break;
      }
      else if (strcmp(pval, "q") == 0) {
	outstream.println();
	outstream.println("Firmware update aborted.");
	return;
      }
    }
  }
  // get firmware file:
  SdFile file;
  SdFile dir;
  if (!dir.open("/")) {
    outstream.printf("! ERROR: Faild to access root folder on SD card.\n");
    outstream.println("Firmware update aborted.");
    return;
  }
  n = 0;
  char hex_file_name[200];
  while (file.openNext(&dir, O_RDONLY)) {
    if (!file.isDir()) {
      file.getName(hex_file_name, 200);
      if (strlen(hex_file_name) > 4 &&
	  strcmp(hex_file_name + strlen(hex_file_name) - 4, ".hex") == 0) {
	if (n == m)
	  break;
	n++;
      }
    }
  }
  outstream.printf("Selected \"%s\" for firmware update\n", hex_file_name);
  if (detailed)
    outstream.println(".");
  else
    outstream.println();
  // check again:
  outstream.println("WARNING: a firmware update could make your device unusable!");
  outstream.println("WARNING: make sure that your device stays powered during the entire firmware update!");
  if (detailed)
    outstream.println(".");
  else
    outstream.println();
  if (!Action::yesno("Do you really want to update the firmware?",
		     false, echo, instream, outstream)) {
    outstream.println();
    outstream.println("Firmware update aborted.");
    return;
  }
  outstream.println();
  // open firmware file:
  File hex_file = sdcard.open(hex_file_name, FILE_READ);
  if (!hex_file) {
    outstream.printf("! ERROR: Failed to open firmware file \"%s\" on SD card.\n",
		     hex_file_name);
    outstream.println();
    outstream.printf("Firmware update aborted.");
    return;
  }
  outstream.printf("Successfully opened firmware file \"%s\".\n",
		   hex_file_name);
  // create flash buffer to hold new firmware:
  outstream.println();
  outstream.println("Updating firmware:");
  outstream.println("- initializing flash buffer ...");
  uint32_t buffer_addr, buffer_size;
  if (firmware_buffer_init( &buffer_addr, &buffer_size ) == 0) {
    outstream.printf("! ERROR: Failed to create flash buffer.\n");
    outstream.println();
    outstream.println("! REBOOT SYSTEM !");
    outstream.println();
    outstream.flush();
    REBOOT;
  }
  outstream.printf("- created flash buffer = %1luK %s (%08lX - %08lX)\n",
		   buffer_size/1024, IN_FLASH(buffer_addr) ? "FLASH" : "RAM",
		   buffer_addr, buffer_addr + buffer_size);
  // read hex file, write new firmware to flash, clean up, reboot
  outstream.println("- updating frimware ...");
  outstream.println();
  update_firmware(&hex_file, &outstream, buffer_addr, buffer_size);
  // return from update_firmware() means error or user abort, so clean up and
  // reboot to ensure that static vars get boot-up initialized before retry
  outstream.println();
  outstream.printf("! ERROR: Failed to update firmware.\n");
  outstream.println();
  outstream.printf("Erase flash buffer / free RAM buffer...\n");
  outstream.println();
  outstream.println("! REBOOT SYSTEM !");
  outstream.flush();
  firmware_buffer_free( buffer_addr, buffer_size );
  REBOOT;
}
