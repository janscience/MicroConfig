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

  /* Initialize top level menu with name and roles. */
  Menu(const char *name, int roles=AllRoles);

  /* Initialize menu with name and roles and add it to menu. */
  Menu(Menu &menu, const char *name, int roles=AllRoles);

  /* Add an action to this Menu. */
  void add(Action *action);

  /* Move action that was already added to this Menu to index. */
  void move(const Action *action, size_t index);

  /* Return the Action matching name. */
  virtual Action *action(const char *name);

  /* Enable the roles of the action matching name. */
  void enable(const char *name, int roles=AllRoles);

  /* Disable the roles of the action matching name. */
  void disable(const char *name, int roles=AllRoles);

  /* Name of the configuration file.
     For the Menu class this simply returns NULL,
     because it does not own a configuration file name.
     See the Config class for a menu with configuration file. */
  virtual const char *configFile() const;

  /* Report name on stream. If descend, also display name and values
     of children. */
  virtual void report(Stream &stream=Serial, size_t indent=0,
		      size_t w=0, bool descend=true) const;

  /* Save current settings to file. */
  virtual void save(File &file, size_t indent=0, size_t w=0) const;

  /* Save current setting to configuration file on SD card.
     The Config class implements this, for the Menu class it simply returns false,
     because it does not own a configuration file name. */
  virtual bool save(Stream &stream=Serial, SDClass *sd=0) const;

  /* Read configuration file from SD card and configure all actions
     accordingly.
     The Config class implements this, for the Menu class it does nothing,
     because it does not own a configuration file name. */
  virtual void load(Stream &stream=Serial, SDClass *sd=0);
  
  /* Interactive menu via serial stream.
     Returns from initial menu after timeout milliseconds.
     If echo, print out received input.
     If detailed provide additional infos for GUI applications. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);

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
