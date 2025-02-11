#include <ConfigurationMenu.h>


ReportConfigAction::ReportConfigAction(Menu &menu, const char *name) :
  Action(menu, name, StreamInput) {
}


void ReportConfigAction::execute(Stream &stream, unsigned long timeout,
				 bool echo, bool detailed) {
  root()->report(stream);
  stream.println();
}


SDClassAction::SDClassAction(Menu &menu, const char *name, SDClass &sd) : 
  Action(menu, name, StreamInput),
  SDC(sd) {
}


void SaveConfigAction::execute(Stream &stream, unsigned long timeout,
			       bool echo, bool detailed) {
  bool save = true;
  if (SDC.exists(root()->configFile())) {
    stream.printf("Configuration file \"%s\" already exists on SD card.\n",
		  root()->configFile());
    save = Action::yesno("Do you want to overwrite the configuration file?",
			 true, echo, stream);
  }
  if (save && root()->save(SDC))
    stream.printf("Saved configuration to file \"%s\" on SD card.\n",
		  root()->configFile());
  stream.println();
}


void LoadConfigAction::execute(Stream &stream, unsigned long timeout,
			       bool echo, bool detailed) {
  if (disabled(StreamInput))
    return;
  if (!SDC.exists(root()->configFile())) {
    stream.printf("Configuration file \"%s\" not found on SD card.\n\n",
		  root()->configFile());
    return;
  }
  stream.println("Reloading the configuration file will discard all changes.");
  bool r = Action::yesno("Do you really want to reload the configuration file?",
			 true, echo, stream);
  stream.println();
  if (r)
    root()->load(SDC);
}


void RemoveConfigAction::execute(Stream &stream, unsigned long timeout,
				 bool echo, bool detailed) {
  if (disabled(StreamInput))
    return;
  if (!SDC.exists(root()->configFile())) {
    stream.printf("Configuration file \"%s\" does not exist on SD card.\n\n",
		  root()->configFile());
    return;
  }
  if (Action::yesno("Do you really want to remove the configuration file?",
		    false, echo, stream)) {
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


ConfigurationMenu::ConfigurationMenu(Menu &menu, SDClass &sd) :
  Menu(menu, "Configuration", Action::StreamInput),
  ReportAct(*this, "Print configuration"),
  SaveAct(*this,"Save configuration", sd),
  LoadAct(*this, "Load configuration", sd),
  RemoveAct(*this, "Erase configuration", sd) {
}

