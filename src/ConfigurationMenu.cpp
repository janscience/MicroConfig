#include <Config.h>
#include <ConfigurationMenu.h>


ReportConfigAction::ReportConfigAction(Menu &menu, const char *name) :
  Action(menu, name) {
}


void ReportConfigAction::execute(Stream &stream) {
  root()->write(stream, FileOutput);
  stream.println();
}


void ReadConfigAction::execute(Stream &stream) {
  if (stream.available() == 0)
    stream.println("Read configuration...");
  elapsedMillis time = 0;
  while ((stream.available() == 0) && (time < 10000)) {
    yield();
    delay(1);
  }
  root()->read(stream, stream);
  stream.println();
}


SDClassAction::SDClassAction(Menu &menu, const char *name, SDClass &sd) : 
  Action(menu, name),
  SDC(sd) {
}


void SaveConfigAction::execute(Stream &stream) {
  bool save = true;
  if (root()->configFile() != NULL && SDC.exists(root()->configFile())) {
    stream.printf("Configuration file \"%s\" already exists on SD card.\n",
		  root()->configFile());
    save = Action::yesno("Do you want to overwrite the configuration file?",
			 true, echo(), stream);
  }
  if (save && root()->save(stream, &SDC))
    stream.printf("Saved configuration to file \"%s\" on SD card.\n",
		  root()->configFile());
  stream.println();
}


void LoadConfigAction::execute(Stream &stream) {
  bool r = true;
  if (root()->configFile() != NULL) {
    if (!SDC.exists(root()->configFile())) {
      stream.printf("Configuration file \"%s\" not found on SD card.\n\n",
		  root()->configFile());
      return;
    }
    stream.println("Reloading the configuration file will discard all changes.");
    r = Action::yesno("Do you really want to reload the configuration file?",
		      true, echo(), stream);
    stream.println();
  }
  if (r)
    root()->load(stream, &SDC);
}


void RemoveConfigAction::execute(Stream &stream) {
  if (root()->configFile() == NULL) {
    stream.println("ERROR! No configuration file name specified.");
    return;
  }
  if (!SDC.exists(root()->configFile())) {
    stream.printf("Configuration file \"%s\" does not exist on SD card.\n\n",
		  root()->configFile());
    return;
  }
  if (Action::yesno("Do you really want to remove the configuration file?",
		    false, echo(), stream)) {
    if (SDC.remove(root()->configFile()))
      stream.printf("\nRemoved configuration file \"%s\" from SD card.\n\n",
		    root()->configFile());
    else
      stream.printf("\nERROR! Failed to remove configuration file \"%s\" from SD card.\n\n",
		    root()->configFile());
  }
  else
    stream.println();
}


StorageAction::StorageAction(Menu &menu, const char *name, Storage &storage) : 
  Action(menu, name),
  Store(storage) {
}


void PutConfigAction::execute(Stream &stream) {
  bool save = Action::yesno("Do you really want to write the configuration settings to EEPROM?",
			    true, echo(), stream);
  if (save)
    root()->put(Store, stream);
  stream.println();
}


void GetConfigAction::execute(Stream &stream) {
  stream.println("Reloading the configuration settings will discard all changes.");
  bool r = Action::yesno("Do you really want to reload the configuration from EEPROM?",
		      true, echo(), stream);
  if (r)
    root()->get(Store, stream);
  stream.println();
}


void StorageClearAction::execute(Stream &stream) {
  if (Action::yesno("Do you really want to clear the full EEPROM memory?", false, echo(), stream)) {
    uint8_t buffer[64];
    memset(buffer, 0xff, 64);
    for (unsigned int i=0; i < Store.length(); i+=64)
      Store.put(i, buffer);
    stream.printf("Wrote 0xFF to all %u EEPROM memory cells.\n",
		  Store.length());
    stream.println();
  }
}


void StorageHexdumpAction::execute(Stream &stream) {
  unsigned int i=0;
  while (i < Store.length()) {
    stream.printf("%04x  ", i);
    uint8_t buffer[16];
    Store.get(i, buffer);
    for (unsigned int j=0; j < 2; j++) {
      for (unsigned int k=0; k < 8; k++) {
	if (i + 8*j + k < Store.length())
	  stream.printf("%02x ", buffer[8*j + k]);
	else
	  stream.print("   ");
      }
      stream.print(" ");
    }
    stream.printf("|");
    for (unsigned int j=0; j < 16 && i < Store.length(); j++) {
      uint8_t c = buffer[j];
      if (c < 0x20 || c >= 0x7f)
	c = '.';
      stream.printf("%c", c);
      i++;
    }
    stream.print("|\n");
  }
  stream.println();
}


ConfigurationMenu::ConfigurationMenu(Menu &menu, SDClass &sd,
				     Storage &storage) :
  Menu(menu, "Configuration", Action::StreamInput),
  ReportAct(*this, "Print configuration"),
  SaveAct(*this,"Save configuration file", sd),
  LoadAct(*this, "Load configuration file", sd),
  RemoveAct(*this, "Erase configuration file", sd),
  PutAct(*this,"Put configuration to EEPROM", storage),
  GetAct(*this, "Get configuration from EEPROM", storage),
  ClearAct(*this, "Clear EEPROM memory", storage),
  HexdumpAct(*this, "EEPROM memory content", storage),
  ReadAct(*this, "Read configuration from stream") {
}

