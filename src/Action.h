/*
  Action - base class for executable or configurable menu entries.
  Created by Jan Benda, November 7, 2023.
*/

#ifndef Action_h
#define Action_h


#include <Arduino.h>


class Menu;
class Config;


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
    EEPROMPut = 64,
    EEPROMGet = 128,
    EEPROMIO = EEPROMPut | EEPROMGet,
    // DisplayUpDownButtons,
    // DisplayTouch,
    // whatever input/output device
    AllRoles = FileIO | StreamIO | EEPROMIO | Report
  };

  /* Ask a yes or no question on a serial I/O stream. */
  static bool yesno(const char *request, bool defval=true,
		    bool echo=true, Stream &stream=Serial);

  /* Initialize action with name and supported roles.
     Warning: only a pointer to name is stored. */
  Action(const char *name, unsigned int roles=AllRoles);

  /* Initialize action with name and supported roles and add it to menu.
     Warning: only a pointer to name is stored.
     This spares memory when passing a literal string.
     If you want the string to be copied, however, then
     use setName(). */
  Action(Menu &menu, const char *name, unsigned int roles=AllRoles);

  /* Destructor. */
  virtual ~Action();

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
  const Config *root() const { return Root; };

  /* The root menu of this action. */
  Config *root() { return Root; };;

  /* Set the root menu of this action. */
  virtual void setRoot(Config *root);

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

  /* set supported roles and current roles to roles. */
  void setRoles(unsigned int roles=AllRoles);

  /* Timeout in milliseconds for interactive menus.
     This implementation returns 0. */
  virtual unsigned long timeOut() const { return 0; };

  /* The number of spaces to be used for each indentation level. */
  size_t indentation() const;

  /* If true, serial input should be printed on output stream. */
  bool echo() const;

  /* If true, more details should be provided in execute(). */
  bool detailed() const;

  /* Write the action's name and potential values or infos to stream
     with proper indentation. roles must be enabled.
     If descend, also display children. */
  virtual void write(Stream &stream=Serial, unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0, bool descend=true) const;
  
  /* Execute this action with user interactions via serial stream.
     StreamInput and StreamOutput must be enabled.
     Returns from menu after timeOut() milliseconds.
     If echo(), print out received input.
     If detailed() provide additional infos for GUI applications.
     Default calls report(stream). */
  virtual void execute(Stream &stream=Serial);

  /* Parse the string val and configure the action accordingly.
     SetValue must be enabled. If StreamOutput is enabled,
     report the new value together with name on stream. */
  virtual void set(const char *val, const char *name=0,
		   Stream &stream=Serial) {};
  
  /* Write configuration with role EEPROMPut to addr in EEPROM memory.
     Report errors and success on stream.
     Returns EEPROM address behind this configuration, -1 on error.
     Default implementation returns addr. */
  virtual int put(int addr, Stream &stream=Serial) const;
  
  /* Read configuration with role EEPROMGet from addr in EEPROM memory.
     Only if setvalue is true set the actions value to EEPROM content.
     Report errors and success on stream.
     Returns EEPROM address behind this configuration, -1 on error.
     Default implementation returns addr. */
  virtual int get(int addr, bool setvalue, Stream &stream=Serial);

  
 protected:

  char *Name;
  unsigned int SupportedRoles;
  unsigned int Roles;

  Menu *Parent;
  Config *Root;
  
};


#endif
