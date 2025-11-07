/*
  Action - base class for executable or configurable menu entries.
  Created by Jan Benda, November 7, 2023.
*/

#ifndef Action_h
#define Action_h


#include <Arduino.h>


class Menu;


class Action {

 public:

  enum Role {
    SetValue = 1,    // set value from a string
    FileOutput = 2,  // write to configuration file
    FileInput = 4,   // read from configuration file
    FileIO = FileInput | FileOutput,
    StreamOutput = 8,
    StreamInput = 16,
    StreamIO = StreamInput | StreamOutput,
    Report = 32,     // write infos to a file
    // DisplayUpDownButtons,
    // DisplayTouch,
    // whatever input/output device
    AllRoles = FileIO | StreamIO | Report
  };

  /* Ask a yes or no question on a serial I/O stream. */
  static bool yesno(const char *request, bool defval=true,
		    bool echo=true, Stream &instream=Serial,
		    Stream &outstream=Serial);

  /* Initialize action with name and supported roles.
     Warning: only a pointer to name is stored. */
  Action(const char *name, unsigned int roles=AllRoles);

  /* Initialize action with name and supported roles and add it to menu.
     Warning: only a pointer to name is stored.
     This spares memory when passing a literal string.
     If you want the string to be copied, however, then
     use setName(). */
  Action(Menu &menu, const char *name, unsigned int roles=AllRoles);

  /* The name identifying the action. */
  const char *name() const { return Name; }

  /* Set the name identifying the action to name.
     In contrast to passing a name to the constructor,
     a string is allocated and the content is copied.
     Note: the destructor does *not* automatically free this allocated memory,
     call clearName() for this. */
  void setName(const char *name);

  /* Free the memory that was allocated for name with setName(). */
  void clearName();

  /* The parent menu of this action. */
  const Menu *parent() const { return Parent; };

  /* The parent menu of this action. */
  Menu *parent() { return Parent; };

  /* Set the parent of this action. */
  void setParent(Menu *parent) { Parent = parent; };

  /* The root menu of this action. */
  const Menu *root() const;

  /* The root menu of this action. */
  Menu *root();

  /* Return this Action if name matches its name. */
  virtual Action *action(const char *name);

  /* True if some of the specified roles are enabled. */
  bool enabled(unsigned int roles=AllRoles) const;

  /* True if the specified roles are not enabled. */
  bool disabled(unsigned int roles=AllRoles) const;

  /* Enable the specified roles for this action, if supported. */
  void enable(unsigned int roles=AllRoles);

  /* Disable the specified roles, if supported. */
  void disable(unsigned int roles=AllRoles);

  /* Disable the specified roles from the supported roles. */
  void disableSupported(unsigned int roles);

  /* Return the roles this action currently has. */
  unsigned int roles() const { return Roles; };
  
  /* Return the roles this action in general supports. */
  unsigned int supportedRoles() const { return SupportedRoles; };

  /* The number of spaces to be used for each indentation level. */
  size_t indentation() const { return Indentation; };

  /* Set the number of spaces to be used for each indentation level. */
  void setIndentation(size_t indentation) { Indentation = indentation; };

  /* Write the action's name and potential values or infos to stream
     with proper indentation. roles must be enabled.
     If descend, also display children. */
  virtual void write(Stream &stream=Serial, unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0, bool descend=true) const;
  
  /* Execute this action with user interactions via serial streams.
     StreamInput and StreamOutput must be enabled.
     Returns from initial menu after timeout milliseconds.
     If echo, print out received input.
     If detailed provide additional infos for GUI applications.
     Default calls report(stream). */
  virtual void execute(Stream &instream=Serial, Stream &outstream=Serial,
		       unsigned long timeout=0, bool echo=true,
		       bool detailed=false);

  /* Parse the string val and configure the action accordingly.
     SetValue must be enabled. If StreamOutput is enabled,
     report the new value together with name on stream. */
  virtual void set(const char *val, const char *name=0,
		   Stream &stream=Serial) {};

  
 protected:

  char *Name;
  unsigned int SupportedRoles;
  unsigned int Roles;

  size_t Indentation;

  Menu *Parent;
  
};


#endif
