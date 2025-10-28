/*
  Config - Menu with configuration file.
  Created by Jan Benda, February 12th, 2025.
*/

#ifndef Config_h
#define Config_h


#include <Menu.h>


class SDClass;


class Config : public Menu {

 public:

  /* Initialize top level menu with name "Menu". */
  Config();

  /* Initialize top level menu with name "Menu" and
     name of configuration file fname on SD card sd. */
  Config(const char *fname, SDClass *sd);

  /* Initialize top level menu with name and roles. */
  Config(const char *name, unsigned int roles=AllRoles);

  /* Name of the configuration file or NULL if not set. */
  virtual const char *configFile() const;

  /* Set name of the configuration file (only a pointer to fname is stored)
     and optional SD card on which to store the config file. */
  void setConfigFile(const char *fname, SDClass *sd=0);

  /* Report name on stream. If descend, also display name and values
     of children. roles must be enabled. */
  virtual void report(Stream &stream, unsigned int roles,
		      size_t indent=0, size_t w=0, bool descend=true) const;

  /* Report configuration menu on stream
     (all actions with FileOutput and Report roles). */
  void report(Stream &stream=Serial) const;

  /* Save current setting to configuration file on SD card
     using the role FileOutput for the report() function.
     Report errors and success on stream.
     Return true on success.
     If sd is NULL write to default SD card provided via setConfigFile(). */
  virtual bool save(Stream &stream=Serial, SDClass *sd=0) const;

  /* Read configuration file from SD card and configure all actions
     accordingly.
     Report errors and success on stream.
     If sd is NULL read from default SD card provided via setConfigFile(). */
  virtual void load(Stream &stream=Serial, SDClass *sd=0);


protected:

  const char *ConfigFile;
  SDClass *SDC;
  
};


#endif
