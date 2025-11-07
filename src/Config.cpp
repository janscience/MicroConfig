#include <SD.h>
#include <Config.h>


Config::Config() :
  Menu("Menu", AllRoles) {
}


Config::Config(const char *fname, SDClass *sd) :
  Config() {
  setConfigFile(fname, sd);
}


Config::Config(const char *name, unsigned int roles) :
  Menu(name, roles) {
}


const char *Config::configFile() const {
  return ConfigFile;
}


void Config::setConfigFile(const char *fname, SDClass *sd) {
  ConfigFile = fname;
  SDC = sd;
}


void Config::report(Stream &stream) const {
  write(stream, FileOutput | Report, 0, 0, true);
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

