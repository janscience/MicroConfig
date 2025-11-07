#include <ConfigurationMenu.h>


ReportConfigAction::ReportConfigAction(Menu &menu, const char *name) :
  Action(menu, name, StreamInput) {
}


void ReportConfigAction::execute(Stream &instream, Stream &outstream,
				 unsigned long timeout, bool echo,
				 bool detailed) {
  root()->write(outstream, FileOutput);
  outstream.println();
}


void ReadConfigAction::execute(Stream &instream, Stream &outstream,
			       unsigned long timeout, bool echo,
			       bool detailed) {
  if (instream.available() == 0)
    outstream.println("Read configuration...");
  elapsedMillis time = 0;
  while ((instream.available() == 0) && (time < 10000)) {
    yield();
    delay(1);
  }
  root()->read(instream, outstream);
  outstream.println();
}


SDClassAction::SDClassAction(Menu &menu, const char *name, SDClass &sd) : 
  Action(menu, name, StreamInput),
  SDC(sd) {
}


void SaveConfigAction::execute(Stream &instream, Stream &outstream,
			       unsigned long timeout, bool echo,
			       bool detailed) {
  bool save = true;
  if (root()->configFile() != NULL && SDC.exists(root()->configFile())) {
    outstream.printf("Configuration file \"%s\" already exists on SD card.\n",
		     root()->configFile());
    save = Action::yesno("Do you want to overwrite the configuration file?",
			 true, echo, instream, outstream);
  }
  if (save && root()->save(outstream, &SDC))
    outstream.printf("Saved configuration to file \"%s\" on SD card.\n",
		     root()->configFile());
  outstream.println();
}


void LoadConfigAction::execute(Stream &instream, Stream &outstream,
			       unsigned long timeout, bool echo,
			       bool detailed) {
  bool r = true;
  if (root()->configFile() != NULL) {
    if (!SDC.exists(root()->configFile())) {
      outstream.printf("Configuration file \"%s\" not found on SD card.\n\n",
		       root()->configFile());
      return;
    }
    outstream.println("Reloading the configuration file will discard all changes.");
    r = Action::yesno("Do you really want to reload the configuration file?",
		      true, echo, instream, outstream);
    outstream.println();
  }
  if (r)
    root()->load(outstream, &SDC);
}


void RemoveConfigAction::execute(Stream &instream, Stream &outstream,
				 unsigned long timeout, bool echo,
				 bool detailed) {
  if (root()->configFile() == NULL) {
    outstream.println("ERROR! No configuration file name specified.");
    return;
  }
  if (!SDC.exists(root()->configFile())) {
    outstream.printf("Configuration file \"%s\" does not exist on SD card.\n\n",
		     root()->configFile());
    return;
  }
  if (Action::yesno("Do you really want to remove the configuration file?",
		    false, echo, instream, outstream)) {
    if (SDC.remove(root()->configFile()))
      outstream.printf("\nRemoved configuration file \"%s\" from SD card.\n\n",
		       root()->configFile());
    else
      outstream.printf("\nERROR! Failed to remove configuration file \"%s\" from SD card.\n\n",
		       root()->configFile());
  }
  else
    outstream.println();
}


ConfigurationMenu::ConfigurationMenu(Menu &menu, SDClass &sd) :
  Menu(menu, "Configuration", Action::StreamInput),
  ReportAct(*this, "Print configuration"),
  SaveAct(*this,"Save configuration file", sd),
  LoadAct(*this, "Load configuration file", sd),
  RemoveAct(*this, "Erase configuration file", sd),
  ReadAct(*this, "Read configuration") {
}

