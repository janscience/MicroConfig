#include <SD.h>
#include <Config.h>


Config::Config() :
  Menu("Menu", ConfigRoles),
  Indentation(4),
  TimeOut(10000),
  Echo(true),
  Detailed(false),
  GUI(false),
  CurrentMode(User),
  ConfigFile(0),
  SDC(0) {
  ActType = MainMenuType;
  Root = this;
}


Config::Config(const char *fname, SDClass *sd) :
  Config() {
  setConfigFile(fname, sd);
}


Config::Config(const char *name, unsigned int roles) :
  Menu(name, roles),
  Indentation(4),
  TimeOut(10000),
  Echo(true),
  Detailed(false),
  GUI(false),
  CurrentMode(User),
  ConfigFile(0),
  SDC(0) {
  ActType = MainMenuType;
  Root = this;
}


void Config::setRoot() {
  Menu::setRoot(this);
}


void Config::setIdentifier() {
  Menu::setIdentifier(0);
}


const char *Config::configFile() const {
  return ConfigFile;
}


void Config::setConfigFile(const char *fname, SDClass *sd) {
  ConfigFile = fname;
  SDC = sd;
}


void Config::execute(Stream &stream) {
  if (disabled(StreamInput))
    return;
  stream.println("\n::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
  Menu::execute(stream);
}


void Config::report(Stream &stream) const {
  write(stream, FileOutput | Report);
}


bool Config::save(Stream &stream, SDClass *sd) const {
  if (sd == NULL)
    sd = SDC;
  if (sd == NULL) {
    stream.println("ERROR! No SD card for saving configuration file specified.");
    return false;
  }
  if (configFile() == NULL) {
    stream.println("ERROR! No configuration file name specified.");
    return false;
  }
  File file = sd->open(configFile(), FILE_WRITE_BEGIN);
  if (!file) {
    stream.printf("ERROR! Configuration file \"%s\" cannot be written to SD card.\n",
		  configFile());
    stream.println("       SD not inserted or SD card full.");
    return false;
  }
  write(file, FileOutput);
  file.close();
  return true;
}


void Config::load(Stream &stream, SDClass *sd) {
  if (sd == NULL)
    sd = SDC;
  if (sd == NULL) {
    stream.println("ERROR! No SD card for saving configuration file specified.");
    return;
  }
  if (configFile() == NULL) {
    stream.println("ERROR! No configuration file name specified.");
    return;
  }
  File file = sd->open(configFile(), FILE_READ);
  if (!file || file.available() < 10) {
    stream.printf("Configuration file \"%s\" not found or empty.\n\n",
		  configFile());
    return;
  }
  stream.printf("Read configuration file \"%s\" ...\n", configFile());
  read(file, stream);
  file.close();
  stream.println();
}


bool Config::put(Storage &storage, Stream &stream) const {
  int start_addr = 0;
  int addr = Menu::put(start_addr, storage, stream);
  if (addr > start_addr) {
    uint32_t crc = storage.crc(start_addr, addr);
    storage.put(addr, crc);
    return true;
  }
  else {
    if (addr < 0)
      stream.println("ERROR! Failed to write settings to storage memory.");
    return false;
  }
}


bool Config::get(Storage &storage, Stream &stream) {
  int start_addr = 0;
  int addr = Menu::get(start_addr, false, storage, stream);
  if (addr > start_addr) {
    uint32_t crc_data = storage.crc(start_addr, addr);
    uint32_t crc_read;
    storage.get(addr, crc_read);
    if (crc_data != crc_read) {
      stream.println("No valid configuration in storage.");
      return false;
    }
    stream.println("Read configuration from storage ...");
    addr = Menu::get(start_addr, true, storage, stream);
    if (addr <= start_addr) {
      stream.println("ERROR! Failed to read settings from storage memory.");
      return false;
    }
    return true;
  }
  return false;
}
