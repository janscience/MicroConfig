/*
  ConfigurationMenu - Actions and menu for managing configurations.
  Created by Jan Benda, February 9th, 2025.
*/

#ifndef ConfigurationMenu_h
#define ConfigurationMenu_h


#include <SD.h>
#include <Action.h>
#include <Menu.h>


class ReportConfigAction : public Action {

 public:

  /* Initialize and add to default menu. */
  ReportConfigAction(const char *name);

  /* Initialize and add to menu. */
  ReportConfigAction(Menu &menu, const char *name);

  /* Report the configuration settings. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);
};


class SDClassAction : public Action {

 public:

  /* Initialize and add to default menu. */
  SDClassAction(const char *name, SDClass &sd);

  /* Initialize and add to menu. */
  SDClassAction(Menu &menu, const char *name, SDClass &sd);

 protected:

  SDClass &SDC; 
};


class SaveConfigAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Save the configuration settings to configuration file. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);
};


class LoadConfigAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Load the configuration settings from configuration file. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);
};


class RemoveConfigAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Remove the configuration file from SD card. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);
};


class ConfigurationMenu : public Menu {

public:

  ConfigurationMenu(SDClass &sd);

protected:

  ReportConfigAction ReportAct;
  SaveConfigAction SaveAct;
  LoadConfigAction LoadAct;
  RemoveConfigAction RemoveAct;
  
};


#endif
