#include <SD.h>
#include <Config.h>


Config::Config() :
  Menu("Menu", AllRoles),
  Indentation(4),
  TimeOut(10000),
  Echo(true),
  Detailed(false),
  GUI(false),
  ConfigFile(0),
  SDC(0) {
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
  ConfigFile(0),
  SDC(0) {
  Root = this;
}


void Config::setRoot() {
  Menu::setRoot(this);
}


const char *Config::configFile() const {
  return ConfigFile;
}


void Config::setConfigFile(const char *fname, SDClass *sd) {
  ConfigFile = fname;
  SDC = sd;
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


uint32_t eeprom_crc(int addr0, int addr1) {
  // Adapted from https://docs.arduino.cc/learn/programming/eeprom-guide/
  
  const uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  if (addr0 < 0)
    addr0 = 0;
  if (addr1 > EEPROM.length())
    addr1 = EEPROM.length();
  uint32_t crc = ~0L;
  for (int index = addr0; index < addr1; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}


int Config::put(Stream &stream) const {
  int start_addr = 0;
  int num = 0;
  int addr = Menu::put(start_addr, num, stream);
  if (addr > start_addr) {
    uint32_t crc = eeprom_crc(start_addr, addr);
    EEPROM.put(addr, crc);
    return num;
  }
  else {
    if (addr < 0)
      stream.println("ERROR! Failed to write settings to EEPROM memory.");
    return addr == start_addr ? 0 : -1;
  }
}


int Config::get(Stream &stream) {
  int start_addr = 0;
  int num = 0;
  int addr = Menu::get(start_addr, num, false, stream);
  if (addr > start_addr) {
    uint32_t crc_data = eeprom_crc(start_addr, addr);
    uint32_t crc_read;
    EEPROM.get(addr, crc_read);
    if (crc_data != crc_read) {
      stream.println("ERROR! EEPROM memory corrupted.");
      return -1;
    }
    num = 0;
    addr = Menu::get(start_addr, num, true, stream);
    if (addr <= start_addr) {
      stream.println("ERROR! Failed to read settings from EEPROM memory.");
      return -1;
    }
    return num;
  }
  return addr == start_addr ? 0 : -1;
}
