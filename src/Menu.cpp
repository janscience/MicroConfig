#include <SD.h>
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


void Menu::add(Action *act) {
  if (NActions >= MaxActions) {
    Serial.printf("ERROR! Number of maximum Actions exceeded in %s!\n",
		  name());
    return;
  }
  Actions[NActions++] = act;
  act->setParent(this);
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
  Serial.printf("found at %d, move to %d, n=%d\n", aidx, index, NActions);
  // move:
  Action *act = Actions[aidx];
  if (index > aidx) {
    for (size_t j=aidx; j<index; j++)
      Actions[j] = Actions[j+1];
  }
  else {
    for (size_t j=aidx; j>index; j--)
      Actions[j] = Actions[j-1];
  }
  Actions[index] = act;
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


const char *Menu::configFile() const {
  return NULL;
}


void Menu::report(Stream &stream, unsigned int roles, size_t indent,
		  size_t w, bool descend) const {
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
      Actions[j]->report(stream, roles, indent, ww, descend);
  }
  else if (enabled(roles) && strlen(name()) > 0)
    stream.printf("%*s%s ...\n", indent, "", name());
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


bool Menu::save(Stream &stream, SDClass *sd) const {
  return false;
}


void Menu::load(Stream &stream, SDClass *sd) {
}


void Menu::execute(Stream &stream, unsigned long timeout,
		   bool echo, bool detailed) {
  if (disabled(StreamInput))
    return;
  int def = 0;
  if (timeout > 0)
    def = -1;
  while (true) {
    stream.printf("%s:\n", name());
    size_t iaction[NActions];
    size_t n = 0;
    for (size_t j=0; j<NActions; j++) {
      if (Actions[j]->enabled(StreamInput)) {
	stream.printf("  %d) ", n+1);
	Actions[j]->report(stream, StreamIO, 0, 0, false);
	iaction[n++] = j;
      }
    }
    while (true) {
      if (def >= 0)
	stream.printf("  Select [%d]: ", def + 1);
      else
	stream.printf("  Select: ");
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
      if (echo)
	stream.println(pval);
      if (strlen(pval) == 0 && def < 0) {
	def = 0;
	continue;
      }
      if (strcmp(pval, "reboot") == 0)
	reboot_board(stream);
      else if (strcmp(pval, "detailed on") == 0)
	detailed = true;
      else if (strcmp(pval, "detailed off") == 0)
	detailed = false;
      else if (strcmp(pval, "echo on") == 0)
	echo = true;
      else if (strcmp(pval, "echo off") == 0)
	echo = false;
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
	  Actions[iaction[i]]->execute(stream, 0, echo, detailed);
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

