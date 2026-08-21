/*
  Menu - A menu of actions and parameters
  Created by Jan Benda, July 8th, 2021.
*/

#ifndef Menu_h
#define Menu_h


#include <Action.h>
#include <Parameter.h>


class Menu : public Action {

 public:

  /* Initialize top level menu with name and roles. 
     FileInput and StorageIO roles are disabled. */
  Menu(const char *name, unsigned int roles=MenuRoles);

  /* Initialize menu with name and roles and add it to menu. 
     FileInput and StorageIO roles are disabled. */
  Menu(Menu &menu, const char *name, unsigned int roles=MenuRoles);

  /* Destructor. */
  virtual ~Menu();

  /* Add an action to this Menu. Sets parent and root of action. */
  void add(Action *action);

  /* Retrun the number of actions contained in this menu. */
  size_t size() const;
  
  /* Return the Action at index idx. */
  Action *operator[](size_t idx);

  /* Recursively set the root menu of this action and all its children. */
  virtual void setRoot(Config *root);
  
  /* Recursively set unique identifiers for the children of this menu. */
  virtual int setIdentifier(int id);

  /* Add a non-editable string parameter to this Menu. */
  ConstStringParameter *addConstString(const char *name,
				       const char *str,
				       Modes mode=User);

  /* Add a string parameter of size N to this Menu. */
  template<int N>
  BaseStringParameter *addString(const char *name, const char *str,
				 Modes mode=User);

  /* Add a string parameter of size N with selection to this Menu. */
  template<int N>
  BaseStringParameter *addString(const char *name, const char *str,
				 const char **selection,
				 size_t n_selection=0,
				 Modes mode=User);
  
  /* Add a boolean parameter to this Menu. */
  BoolParameter *addBoolean(const char *name, bool value,
			    Modes mode=User);
  
  /* Add a integer parameter to this Menu. */
  NumberParameter<int> *addInteger(const char *name, int value,
				   const char *unit=0,
				   Modes mode=User);
  
  /* Add a integer parameter to this Menu. */
  NumberParameter<int> *addInteger(const char *name, int value,
				   int minimum, int maximum,
				   const char *unit=0,
				   const char *outunit=0,
				   Modes mode=User);
  
  /* Add a float parameter to this Menu. */
  NumberParameter<float> *addFloat(const char *name, float value,
				   const char *format="%g",
				   const char *unit=0,
				   Modes mode=User);
  
  /* Add a float parameter to this Menu. */
  NumberParameter<float> *addFloat(const char *name, float value,
				   float minimum, float maximum,
				   const char *format="%g",
				   const char *unit=0,
				   const char *outunit=0,
				   Modes mode=User);

  /* Move action that was already added to this Menu to new position
     index. */
  void move(const Action *action, size_t index);

  /* Return the Action matching name. */
  virtual Action *action(const char *name);

  /* Return the Action whose identifier matches id. */
  virtual Action *action(int id);

  /* Enable the specified roles for this menu, if supported. */
  void enable(unsigned int roles=AllRoles);

  /* Disable the specified roles for this menu, if supported. */
  void disable(unsigned int roles=AllRoles);
  
  /* Enable the roles of the action matching name. */
  void enable(const char *name, unsigned int roles=AllRoles);

  /* Disable the roles of the action matching name. */
  void disable(const char *name, unsigned int roles=AllRoles);

  /* Write the menu's name followed by '...' to stream
     for display as a menu entry. */
  virtual void writeEntry(Stream &stream=Serial, size_t width=0) const;
  
  /* Write name to stream. If descend, also display name and values
     of children. roles must be enabled. */
  virtual void write(Stream &stream=Serial,
		     unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0) const;

  /* Read configuration settings from instream as long as data are
     available or a line starting with "DONE" is encountered, and
     report errors on outstream. */
  virtual void read(Stream &instream=Serial, Stream &outstream=Serial);
  
  /* Interactive menu via serial stream. */
  virtual void execute(Stream &stream=Serial);

  /* Set the provided name-value pair and report on stream. */
  virtual void set(const char *val, const char *name,
		   Stream &stream=Serial);

  /* Write configuration with role StoragePut to addr in storage memory.
     Returns address behind this configuration, -1 on error.
     Report errors and success on stream. */
  virtual int put(int addr, Storage &storage, Stream &stream=Serial) const;
  
  /* Read configuration with role StorageGet from addr in storage memory.
     Only if setvalue is true set the action's value to content from storage.
     Returns address behind this configuration, -1 on error.
     Report errors and success on stream. */
  virtual int get(int addr, bool setvalue,
		  Storage &storage, Stream &stream=Serial);

  /* Recursively transmit configuration of all children using storage.
     Returns a negative number on error, zero if nothing was
     transmitted, or the number of transmitted actions on success. */
  virtual int transmit(Storage &storage, Stream &stream=Serial) const;

  /* Receive identifier and value from storage and update the
     corresponding action's value accordingly.
     Returns a negative number on error, zero on stop,
     or the identifier of the updated action on success. */
  virtual int receive(Storage &storage, Stream &stream=Serial);

protected:

  static const size_t MaxActions = 16;
  size_t NActions;
  Action *Actions[MaxActions];
  bool Own[MaxActions];
  bool GoHome;
  
};


template<int N>
BaseStringParameter *Menu::addString(const char *name, const char *str,
				     Modes mode) {
  if ((name == 0) || (str == 0))
    return 0;
  StringParameter<N> *act = new StringParameter<N>(*this, name, str,
						   mode);
  Own[NActions - 1] = true;
  return act;
}


template<int N>
BaseStringParameter *Menu::addString(const char *name, const char *str,
				     const char **selection,
				     size_t n_selection,
				     Modes mode) {
  if ((name == 0) || (str == 0))
    return 0;
  StringParameter<N> *act = new StringParameter<N>(*this, name, str,
						   selection,
						   n_selection,
						   mode);
  Own[NActions - 1] = true;
  return act;
}


#endif
