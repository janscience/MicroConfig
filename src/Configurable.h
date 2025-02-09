/*
  Configurable - A collection of Actions
  Created by Jan Benda, July 8th, 2021.
*/

#ifndef Configurable_h
#define Configurable_h


#include <SD.h>
#include <Action.h>


class Configurable : public Action {

 public:

  /* Initialize configuration section name and add it to default menu. */
  Configurable(const char *name, int roles=AllRoles);

  /* Initialize configuration section name and add it to menu. */
  Configurable(Configurable &menu, const char *name, int roles=AllRoles);

  /* Add an action to this Configurable. */
  void add(Action *action);

  /* Move action that was already added to this Configurable to index. */
  void move(const Action *action, size_t index);

  /* Return the Action matching name. */
  virtual Action *action(const char *name);

  /* Enable the roles of the action matching name. */
  void enable(const char *name, int roles=AllRoles);

  /* Disable the roles of the action matching name. */
  void disable(const char *name, int roles=AllRoles);

  /* Report name on stream. If descend, also display name and values
     of children. */
  virtual void report(Stream &stream=Serial, size_t indent=0,
		      size_t w=0, bool descend=true) const;

  /* Save current settings to file. */
  virtual void save(File &file, size_t indent=0, size_t w=0) const;

  /* Save current setting to configuration file on SD card.
     Return true on success. */
  bool save(SDClass &sd, const char *filename) const;

  /* Read configuration file from SD card and configure all actions
     accordingly. */
  void load(SDClass &sd, const char *filename);
  
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

  static const size_t MaxActions = 32;
  size_t NActions;
  Action *Actions[MaxActions];

};


#endif
