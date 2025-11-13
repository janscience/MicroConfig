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

  /* Initialize and add to menu. */
  ReportConfigAction(Menu &menu, const char *name);

  /* Report the configuration settings. */
  virtual void execute(Stream &stream=Serial);
};


class ReadConfigAction : public ReportConfigAction {

 public:
  
  using ReportConfigAction::ReportConfigAction;

  /* Read configuration settings from stream. */
  virtual void execute(Stream &stream=Serial);
};


class SDClassAction : public Action {

 public:

  /* Initialize and add to menu. */
  SDClassAction(Menu &menu, const char *name, SDClass &sd);

 protected:

  SDClass &SDC; 
};


class SaveConfigAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Save the configuration settings to configuration file. */
  virtual void execute(Stream &stream=Serial);
};


class LoadConfigAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Load the configuration settings from configuration file. */
  virtual void execute(Stream &stream=Serial);
};


class RemoveConfigAction : public SDClassAction {

 public:

  using SDClassAction::SDClassAction;

  /* Remove the configuration file from SD card. */
  virtual void execute(Stream &stream=Serial);
};


class PutConfigAction : public ReportConfigAction {

 public:
  
  using ReportConfigAction::ReportConfigAction;

  /* Write configuration settings to EEPROM. */
  virtual void execute(Stream &stream=Serial);
};


class GetConfigAction : public ReportConfigAction {

 public:
  
  using ReportConfigAction::ReportConfigAction;

  /* Read configuration settings from EEPROM. */
  virtual void execute(Stream &stream=Serial);
};


class ConfigurationMenu : public Menu {

public:

  ConfigurationMenu(Menu &menu, SDClass &sd);

  ReportConfigAction ReportAct;
  SaveConfigAction SaveAct;
  LoadConfigAction LoadAct;
  RemoveConfigAction RemoveAct;
  PutConfigAction PutAct;
  GetConfigAction GetAct;
  ReadConfigAction ReadAct;
  
};


#endif
