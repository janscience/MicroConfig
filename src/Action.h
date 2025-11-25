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

  // roles an action supports:
  enum Role {
    SetValue = 1,     // set value from a string using set() function.
    FileOutput = 2,   // write to configuration file using write() function.
    FileInput = 4,    // read from configuration file using set() function.
    FileIO = FileInput | FileOutput,
    StreamOutput = 8, // report infos on stream using write() function.
    StreamInput = 16, // interactive action supporting execute() function.
    StreamIO = StreamInput | StreamOutput,
    Report = 32,      // write infos to a file using write() function.
    EEPROMPut = 64,   // write to EEPROM using put() function.
    EEPROMGet = 128,  // read from EEPROM using get() function.
    EEPROMIO = EEPROMPut | EEPROMGet,
    ActionRoles = StreamInput,         // Action that can execute
    ReportRoles = StreamIO | Report,   // Action that can execute and report
    ParameterRoles = SetValue | FileIO | StreamIO | EEPROMIO | Report,
    ConstParameterRoles = StreamOutput | Report,
    MenuRoles = FileIO | StreamIO | Report,
    ConfigRoles = StreamIO,
    AllRoles = FileIO | StreamIO | EEPROMGet | Report  // EEPROMPut must no be touched to keep EEPROM memory intact when disabling or enabling actions
  };

  enum Modes {
    Admin = 1,  // administration mode for actions that should not be visible for normal users
    User = 2,   // user mode for normal actions the user should see
    AllModes = Admin | User
  };

  enum ActionTypes {
    ActionType = 1,      // Action is neither a Parameter nor a Menu.
    ParameterType = 2,   // Action is a Parameter.
    MenuType = 4,        // Action is a Menu.
    MainMenuType = 12    // Action is the main menu (Config class).
  };

  /* Ask a yes or no question on a serial I/O stream. */
  static bool yesno(const char *request, bool defval=true,
		    bool echo=true, Stream &stream=Serial);

  /* Initialize action with name and supported roles and mode.
     Warning: only a pointer to name is stored.
     That is, name must be a static character array.
     This spares memory when passing a literal string.
     If you want the string to be copied, however, then
     use setName(). */
  Action(const char *name, unsigned int roles=ActionRoles,
	 Modes mode=AllModes);

  /* Initialize action with name and supported roles and mode
     and add it to menu.
     Warning: only a pointer to name is stored.
     That is, name must be a static character array.
     This spares memory when passing a literal string.
     If you want the string to be copied, however, then
     use setName(). */
  Action(Menu &menu, const char *name, unsigned int roles=ActionRoles,
	 Modes mode=AllModes);

  /* Destructor. */
  virtual ~Action();

  /* The type of this action. */
  ActionTypes actionType() const { return ActType; };

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

  /* Set supported roles and current roles to roles. */
  void setRoles(unsigned int roles=AllRoles);

  /* Return modes supported by this action. */
  Modes mode() const { return Mode; };

  /* Set modes supported by this action to mode. */
  void setMode(Modes mode) { Mode = mode; };

  /* Timeout in milliseconds for interactive menus.
     This implementation returns 0. */
  virtual unsigned long timeOut() const { return 0; };

  /* The number of spaces to be used for each indentation level. */
  size_t indentation() const;

  /* If true, serial input should be printed on output stream. */
  bool echo() const;

  /* If true, more details should be provided in execute(). */
  bool detailed() const;

  /* If true, a GUI is operating the interactive menu. */
  bool gui() const;

  /* Return the current mode of the interactive menu (Admin or User). */
  Modes currentMode() const;

  /* Write the action's name and potential values or infos to stream
     for display as a menu entry.
     Just a one-liner with a single newline at the end.
     Called from Menu::execute() when StreamInput is enabled for this action.
     You usually do not need to reimplement this function, since
     Parameter and Menu actions do already the right thing. */
  virtual void writeEntry(Stream &stream=Serial, size_t width=0) const;

  /* Write some infos to stream with proper indentation.
     The stream can be, for example, a file or output to a console.
     This function is only called, if this Action has roles enabled.
     However, an implementation might take roles into account (roles
     can be StreamOutput, FileOutput or Report).
     This default implementation does nothing. */
  virtual void write(Stream &stream=Serial, unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0) const;
  
  /* Execute this action with user interactions via serial stream.
     StreamInput and StreamOutput must be enabled.
     Returns from menu after timeOut() milliseconds.
     If echo(), print out received input.
     If detailed() provide additional infos for GUI applications.
     Default calls write(stream, StreamOutput). */
  virtual void execute(Stream &stream=Serial);

  /* Parse the string val and configure the action accordingly.
     SetValue must be enabled. If StreamOutput is enabled,
     report the new value together with name on stream. */
  virtual void set(const char *val, const char *name=0,
		   Stream &stream=Serial) {};
  
  /* Write configuration with role EEPROMPut to addr in EEPROM memory.
     num is the current index of this Action.
     It is incremented when this Action stores values in EEPROM.
     Report errors and success on stream.
     Returns EEPROM address behind this configuration, -1 on error.
     Default implementation returns addr. */
  virtual int put(int addr, int &num, Stream &stream=Serial) const;
  
  /* Read configuration with role EEPROMGet from addr in EEPROM memory.
     num is the current index of this Action.
     It is incremented when this Action stores values in EEPROM.
     Only if setvalue is true set the actions value to EEPROM content.
     Report errors and success on stream.
     Returns EEPROM address behind this configuration, -1 on error.
     Default implementation returns addr. */
  virtual int get(int addr, int &num, bool setvalue, Stream &stream=Serial);

  
 protected:

  ActionTypes ActType;
  Modes Mode;
  char *Name;
  unsigned int SupportedRoles;
  unsigned int Roles;

  Menu *Parent;
  Config *Root;
  
};


#endif
