/*
  FirmwareMenu - Actions and menu for handling firmware updates.
  Created by Jan Benda, August 13th, 2024.
*/

#ifndef FirmwareMenu_h
#define FirmwareMenu_h


#include <SD.h>
#include <Menu.h>
#include <ConfigurationMenu.h>


class ListFirmwareAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* List firmware hex files found on SD card */
  virtual void execute(Stream &instream=Serial, Stream &outstream=Serial,
		       unsigned long timeout=0, bool echo=true,
		       bool detailed=false);
};


class UpdateFirmwareAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Upload firmware from SD card */
  virtual void execute(Stream &instream=Serial, Stream &outstream=Serial,
		       unsigned long timeout=0, bool echo=true,
		       bool detailed=false);
};


class FirmwareMenu : public Menu {

public:

  FirmwareMenu(Menu &menu, SDClass &sdcard);

  ListFirmwareAction ListAct;
  UpdateFirmwareAction UpdateAct;
  
};


#endif
