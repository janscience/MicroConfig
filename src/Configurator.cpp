#include <Configurator.h>


Configurator *Configurator::MainConfig = NULL;


Configurator::Configurator() :
  Configurator("Menu") {
}


Configurator::Configurator(const char *name) :
  Menu(name),
  Config(this) {
  MainConfig = this;
  strncpy(ConfigFile, "micro.cfg", MaxFile);
  ConfigFile[MaxFile-1] = '\0';
  disableSupported(StreamOutput);
  disableSupported(FileIO);
}


void Configurator::setConfigFile(const char *fname) {
  strncpy(ConfigFile, fname, MaxFile);
  ConfigFile[MaxFile-1] = '\0';
}


void Configurator::report(Stream &stream, size_t indent,
			  size_t w, bool descend) const {
  Config->Menu::report(stream, indent, w, descend);
}


bool Configurator::save(SDClass &sd) const {
  return Config->Menu::save(sd, ConfigFile);
}


void Configurator::load(SDClass &sd) {
  Config->Menu::load(sd, ConfigFile);
}


void Configurator::execute(Stream &stream, unsigned long timeout,
			   bool echo, bool detailed) {
  Menu::execute(stream, timeout, echo, detailed);
}

