#include <Config.h>
#include <Menu.h>


void reboot_board(Stream &stream) {
  stream.println();
  stream.println("REBOOT NOW!");
  delay(10);
#if defined(__IMXRT1062__) // Teensy 4.0/4.1
  SCB_AIRCR = 0x05FA0004;
#else
  // from https://github.com/joepasquariello/FlasherX :
  // reboot is the same for all ARM devices
  #define CPU_RESTART_ADDR        ((uint32_t *)0xE000ED0C)
  #define CPU_RESTART_VAL         (0x5FA0004)
  #define REBOOT                  (*CPU_RESTART_ADDR = CPU_RESTART_VAL)
#endif
}


Menu::Menu(const char *name, unsigned int roles) :
  Action(name, roles),
  NActions(0),
  GoHome(false) {
  disableSupported(StreamOutput);
  disableSupported(FileIO);
  disableSupported(Report);
}


Menu::Menu(Menu &menu, const char *name, unsigned int roles) :
  Action(menu, name, roles),
  NActions(0),
  GoHome(false) {
}


Menu::~Menu() {
  for (size_t j=0; j<NActions; j++) {
    if (Own[j])
      delete Actions[j];
  }
}


void Menu::add(Action *act) {
  if (NActions >= MaxActions) {
    Serial.printf("ERROR! Number of maximum Actions exceeded in %s!\n",
		  name());
    return;
  }
  Own[NActions] = false;
  Actions[NActions++] = act;
  act->setParent(this);
  act->setRoot(Root);
}


void Menu::setRoot(Config *root) {
  Root = root;
  for (size_t j=0; j<NActions; j++)
    Actions[j]->setRoot(root);
}


ConstStringParameter *Menu::addConstString(const char *name,
					   const char *str,
					   unsigned int roles) {
  if ((name == 0) || (str == 0))
    return 0;
  ConstStringParameter *act = new ConstStringParameter(*this, name, str);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}


BoolParameter *Menu::addBoolean(const char *name, bool value,
				unsigned int roles) {
  if (name == 0)
    return 0;
  BoolParameter *act = new BoolParameter(*this, name, value);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}

  
NumberParameter<int> *Menu::addInteger(const char *name, int value,
				       const char *unit, unsigned int roles) {
  if (name == 0)
    return 0;
  NumberParameter<int> *act = new NumberParameter<int>(*this, name, value,
						       "%d", unit);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}

  
NumberParameter<int> *Menu::addInteger(const char *name, int value,
				       int minimum, int maximum,
				       const char *unit, const char *outunit,
				       unsigned int roles) {
  if (name == 0)
    return 0;
  NumberParameter<int> *act = new NumberParameter<int>(*this, name, value,
						       minimum, maximum,
						       "%d", unit, outunit);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}

  
NumberParameter<float> *Menu::addFloat(const char *name, float value,
				       const char *format, const char *unit,
				       unsigned int roles) {
  if (name == 0)
    return 0;
  NumberParameter<float> *act = new NumberParameter<float>(*this, name, value,
							   format, unit);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}

  
NumberParameter<float> *Menu::addFloat(const char *name, float value,
				       float minimum, float maximum,
				       const char *format, const char *unit,
				       const char *outunit,
				       unsigned int roles) {
  if (name == 0)
    return 0;
  NumberParameter<float> *act = new NumberParameter<float>(*this, name, value,
							   minimum, maximum,
							   format,
							   unit, outunit);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}


void Menu::move(const Action *action, size_t index) {
  Serial.println("MOVE");
  if (index >= NActions)
    return;
  // find index of action:
  size_t aidx = 0;
  for (size_t j=0; j<NActions; j++) {
    if (Actions[j] == action) {
      aidx = j;
      break;
    }
  }
  if (Actions[aidx] != action)
    return;
  if (aidx == index)
    return;
  // move:
  Action *act = Actions[aidx];
  bool own = Own[aidx];
  if (index > aidx) {
    for (size_t j=aidx; j<index; j++) {
      Actions[j] = Actions[j+1];
      Own[j] = Own[j+1];
    }
  }
  else {
    for (size_t j=aidx; j>index; j--) {
      Actions[j] = Actions[j-1];
      Own[j] = Own[j-1];
    }
  }
  Actions[index] = act;
  Own[index] = own;
}


Action *Menu::action(const char *name) {
  size_t inx = strlen(name);
  char lname[strlen(name)+1];
  for (size_t k=0; k<strlen(name)+1; k++) {
    if (name[k] == '>') {
      lname[k] = '\0';
      inx = k + 1;
      break;
    }
    lname[k] = tolower(name[k]);
  }
  for (size_t j=0; j<NActions; j++) {
    char cname[strlen(Actions[j]->name())+1];
    for (size_t k=0; k<strlen(Actions[j]->name())+1; k++)
      cname[k] = tolower(Actions[j]->name()[k]);
    if (strcmp(cname, lname) == 0) {
      if (inx < strlen(name))
	return Actions[j]->action(name + inx);
      else
	return Actions[j];
    }
  }
  return NULL;
}


void Menu::enable(unsigned int roles) {
  Action::enable(roles);
}


void Menu::disable(unsigned int roles) {
  Action::disable(roles);
}


void Menu::enable(const char *name, unsigned int roles) {
  Action *act = action(name);
  if (act != NULL)
    act->enable(roles);
}


void Menu::disable(const char *name, unsigned int roles) {
  Action *act = action(name);
  if (act != NULL)
    act->disable(roles);
}


void Menu::write(Stream &stream, unsigned int roles, size_t indent,
		 size_t width, bool descend) const {
  // write actions to serial:
  if (descend) {
    if (enabled(roles) && strlen(name()) > 0) {
      stream.printf("%*s%s:\n", indent, "", name());
      indent += indentation();
    }
    // longest name:
    size_t ww = 0;
    for (size_t j=0; j<NActions; j++) {
      if (Actions[j]->enabled(roles) && strlen(Actions[j]->name()) > ww)
	ww = strlen(Actions[j]->name());
    }
    for (size_t j=0; j<NActions; j++)
      Actions[j]->write(stream, roles, indent, ww, descend);
  }
  else if (enabled(roles) && strlen(name()) > 0) {
    size_t n = 0;
    for (size_t j=0; j<NActions; j++) {
      if (Actions[j]->enabled(StreamInput))
	n++;
    }
    if (n > 0)
      stream.printf("%*s%s ...\n", indent, "", name());
    else
      stream.printf("%*s%s\n", indent, "", name());
  }
}


void Menu::read(Stream &instream, Stream &outstream) {
  Action *act = NULL;
  const size_t nline = 128;
  char line[nline];
  char sections[nline];
  sections[0] = '\0';
  int indent = 0;
  int previndent = -1;
  int nind = -1;
  while (instream.available()) {
    instream.readBytesUntil('\n', line, nline);
    if (strncmp(line, "DONE", 4) == 0)
      break;
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
	  outstream.printf("  no configuration candidate for section \"%s\" found.\n", sections);
      }
      else if (act) {
	for (int i=strlen(val)-1; i>=0; i--) {
	  if (val[i] != ' ') {
	    val[i+1] = '\0';
	    break;
	  }
	}
	act->set(val, key, outstream);
      }
    }
    if (instream.available() == 0)
      delay(10);
  }
}


void Menu::execute(Stream &stream) {
  if (disabled(StreamInput))
    return;
  unsigned long timeout = timeOut();
  int def = 0;
  if (timeout > 0)
    def = -1;
  while (true) {
    stream.printf("%s:\n", name());
    // count interactive entries:
    size_t nn = 0;
    size_t width = 0;
    for (size_t j=0; j<NActions; j++) {
      if (Actions[j]->enabled(StreamInput))
	nn++;
      if (Actions[j]->enabled(StreamIO)) {
	if (width < strlen(Actions[j]->name()))
	  width = strlen(Actions[j]->name());
      }
    }
    // list entries:
    size_t iaction[NActions];
    size_t n = 0;
    for (size_t j=0; j<NActions; j++) {
      if (Actions[j]->enabled(StreamIO)) {
	stream.printf("%*s", indentation(), "");
	if (Actions[j]->enabled(StreamInput)) {
	  stream.printf("%d) ", n+1);
	  iaction[n++] = j;
	}
	else if (nn > 0)
	  stream.printf("%*s", n < 10 ? 3 : 4, "");
	Actions[j]->write(stream, StreamIO, 0, width, false);
      }
    }
    if (n == 0) {
      stream.println();
      break;
    }
    while (true) {
      stream.print("Select");
      if (def >= 0)
	stream.printf(" [%d]: ", def + 1);
      else
	stream.printf(": ");
      elapsedMillis time = 0;
      while ((stream.available() == 0) && (timeout == 0 || time < timeout)) {
	yield();
	delay(1);
      }
      if (stream.available() == 0) {
	// timeout:
	stream.println('\n');
	stream.printf("Timeout! Exit %s now.\n\n", name());
	return;
      }
      timeout = 0;
      char pval[32];
      stream.readBytesUntil('\n', pval, 32);
      if (strlen(pval) == 0 && def >= 0)
	sprintf(pval, "%d", def + 1);
      if (echo())
	stream.println(pval);
      if (strlen(pval) == 0 && def < 0) {
	def = 0;
	continue;
      }
      if (strcmp(pval, "show") == 0) {
	stream.println();
	stream.println("Menu settings:");
	stream.printf("%*sdetailed:    %s\n", indentation(), "",
		      Root->detailed() ? "on" : "off");
	stream.printf("%*secho:        %s\n", indentation(), "",
		      Root->echo() ? "on" : "off");
	stream.printf("%*sindentation: %d\n", indentation(), "",
		      Root->indentation());
	stream.printf("%*stimeout:     %.3fs\n", indentation(), "",
		      0.001*Root->timeOut());
	stream.println();
	break;
      }
      else if (strcmp(pval, "reboot") == 0)
	reboot_board(stream);
      else if (strcmp(pval, "detailed on") == 0)
	Root->setDetailed(true);
      else if (strcmp(pval, "detailed off") == 0)
	Root->setDetailed(false);
      else if (strcmp(pval, "echo on") == 0)
	Root->setEcho(true);
      else if (strcmp(pval, "echo off") == 0)
	Root->setEcho(false);
      else if (strcmp(pval, "print") == 0) {
	stream.println();
	break;
      }
      else {
	char *end;
	long i = strtol(pval, &end, 10) - 1;
	if (end != pval && i >= 0 && i < (long)n &&
	    iaction[i] < NActions) {
	  def = i;
	  stream.println();
	  Actions[iaction[i]]->execute(stream);
	  if (root()->GoHome) {
	    if (this != root()) {
	      // go up one level:
	      return;
	    }
	    else {
	      // top-level menu reached, stay and unmark:
	      root()->GoHome = false;
	    }
	  }
	  break;
	}
	else if (strcmp(pval, "h") == 0) {
	  if (this != root()) {
	    // mark to go home and go up one level:
	    stream.println();
	    root()->GoHome = true;
	    return;
	  }
	  else {
	    // already at top-level menu, reprint:
	    stream.println();
	    break;
	  }
	}
	else if (strcmp(pval, "q") == 0) {
	  // exit this menu:
	  stream.println();
	  return;
	}
      }
    }
  }
}


void Menu::set(const char *val, const char *name,
	       Stream &stream) {
  Action *act = action(name);
  if (act == NULL) {
    if (enabled(StreamOutput))
	stream.printf("%*s%s name \"%s\" not found.\n",
		      indentation(), "", this->name(), name);
  }
  else
    act->set(val, this->name());
}


int Menu::put(int addr, int &num, Stream &stream) const {
  for (size_t j=0; j<NActions; j++) {
    addr = Actions[j]->put(addr, num, stream);
    if (addr < 0)
      return addr;
  }
  return addr;
}


int Menu::get(int addr, int &num, bool setvalue, Stream &stream) {
  for (size_t j=0; j<NActions; j++) {
    addr = Actions[j]->get(addr, num, setvalue, stream);
    if (addr < 0)
      return addr;
  }
  return addr;
}

