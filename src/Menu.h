/*
  Menu - A menu of actions and parameters
  Created by Jan Benda, July 8th, 2021.
*/

#ifndef Menu_h
#define Menu_h


#include <Action.h>


class SDClass;


class Menu : public Action {

 public:

  /* Initialize top level menu with name and roles. 
     StreamOutput, FileIO, and Report are disabled. */
  Menu(const char *name, unsigned int roles=AllRoles);

  /* Initialize menu with name and roles and add it to menu. */
  Menu(Menu &menu, const char *name, unsigned int roles=AllRoles);

  /* Add an action to this Menu. */
  void add(Action *action);

  /* Move action that was already added to this Menu to index. */
  void move(const Action *action, size_t index);

  /* Return the Action matching name. */
  virtual Action *action(const char *name);

  /* Enable the specified roles for this menu, if supported. */
  void enable(unsigned int roles=AllRoles);

  /* Disable the specified roles for this menu, if supported. */
  void disable(unsigned int roles=AllRoles);
  
  /* Enable the roles of the action matching name. */
  void enable(const char *name, unsigned int roles=AllRoles);

  /* Disable the roles of the action matching name. */
  void disable(const char *name, unsigned int roles=AllRoles);

  /* Name of the configuration file.
     For the Menu class this simply returns NULL,
     because it does not own a configuration file name.
     See the Config class for a menu with configuration file. */
  virtual const char *configFile() const;

  /* Report name on stream. If descend, also display name and values
     of children. roles must be enabled. */
  virtual void write(Stream &stream=Serial, unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0, bool descend=true) const;

  /* Read configuration settings from instream as long as data are available
     or a line starting with "DONE" is encountered, and
     report errors on outstream. */
  virtual void read(Stream &instream=Serial, Stream &outstream=Serial);

  /* Save current setting to configuration file on SD card.
     The Config class implements this, for the Menu class it simply returns false,
     because it does not own a configuration file name. */
  virtual bool save(Stream &stream=Serial, SDClass *sd=0) const;

  /* Read configuration file from SD card and configure all actions
     accordingly.
     The Config class implements this, for the Menu class it does nothing,
     because it does not own a configuration file name. */
  virtual void load(Stream &stream=Serial, SDClass *sd=0);
  
  /* Interactive menu via serial streams.
     Returns from initial menu after timeout milliseconds.
     If echo, print out received input.
     If detailed provide additional infos for GUI applications. */
  virtual void execute(Stream &instream=Serial, Stream &outstream=Serial,
		       unsigned long timeout=0, bool echo=true,
		       bool detailed=false);

  /* Set the provided name-value pair and report on stream. */
  virtual void set(const char *val, const char *name,
		   Stream &stream=Serial);


protected:

  static const size_t MaxActions = 16;
  size_t NActions;
  Action *Actions[MaxActions];
  bool GoHome;
  
};


#endif
