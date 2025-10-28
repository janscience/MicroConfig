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


void Config::report(Stream &stream, unsigned int roles,
		    size_t indent, size_t w, bool descend) const {
  Menu::report(stream, roles, indent, w, descend);
}


void Config::report(Stream &stream) const {
  report(stream, FileOutput | Report, 0, 0, true);
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
  report(file, FileOutput);
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
  Action *act = NULL;
  const size_t nline = 128;
  char line[nline];
  char sections[nline];
  File file = sd->open(configFile(), FILE_READ);
  if (!file || file.available() < 10) {
    stream.printf("Configuration file \"%s\" not found or empty.\n\n",
		  configFile());
    return;
  }
  stream.printf("Read configuration file \"%s\" ...\n", configFile());
  sections[0] = '\0';
  int indent = 0;
  int previndent = -1;
  int nind = -1;
  while (file.available()) {
    file.readBytesUntil('\n', line, nline);
    char *key = NULL;
    char *val = NULL;
    int state = 0;
    for (size_t k=0; line[k] != '\0'; k++) {
      if (line[k] == '#') {
	line[k] = '\0';
	break;
      }
      if (line[k] == '\n' || line[k] == '\r')
	line[k] = ' ';
      switch (state) {
      case 0: if (line[k] != ' ') {
	  indent = k;
	  if (previndent < 0)
	    previndent = indent;
	  line[k] = tolower(line[k]);
	  key = &line[k];
	  state++;
	}
	break;
      case 1: line[k] = tolower(line[k]);
	if (line[k] == ':') {
	  line[k] = '\0';
	  state++;
	  for (int i=k-1; i>=0; i--) {
	    if (line[i] != ' ') {
	      line[i+1] = '\0';
	      break;
	    }
	  }
	}
	break;
      case 2: if (line[k] != ' ') {
	  val = &line[k];
	  state++;
	}
	break;
      }
    }
    if (state > 1) {
      if (val == NULL) {
	if (indent > previndent) {
	  if (nind < 0)
	    nind = indent - previndent;
	  if (strlen(sections) > 0)
	    strcat(sections, ">");
	  strcat(sections, key);
	}
	else {
	  int n = previndent - indent;
	  n /= nind >= 0 ? nind : 2;
	  n += 1;
	  // n sections up:
	  for(int i=strlen(sections)-1; i>=0; i--) {
	    if (sections[i] == '>') {
	      sections[i] = '\0';
	      if (--n == 0)
		break;
	    }
	  }
	  if (n > 0)
	    sections[0] = '\0';
	  // add new section:
	  if (strlen(sections) > 0)
	    strcat(sections, ">");
	  strcat(sections, key);
	}
	previndent = indent;
	act = action(sections);
	if (act == NULL)
	  stream.printf("  no configuration candidate for section \"%s\" found.\n", sections);
      }
      else if (act) {
	for (int i=strlen(val)-1; i>=0; i--) {
	  if (val[i] != ' ') {
	    val[i+1] = '\0';
	    break;
	  }
	}
	act->set(val, key, stream);
      }
    }
  }
  file.close();
  stream.println();
}

