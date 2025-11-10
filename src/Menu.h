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
     StreamOutput, FileIO, and Report are disabled. */
  Menu(const char *name, unsigned int roles=AllRoles);

  /* Initialize menu with name and roles and add it to menu. */
  Menu(Menu &menu, const char *name, unsigned int roles=AllRoles);

  /* Destructor. */
  virtual ~Menu();

  /* Add an action to this Menu. Sets parent and root of action. */
  void add(Action *action);

  /* Set the root menu of this action and all its children. */
  virtual void setRoot(Config *root);

  /* Add a non-editable string parameter to this Menu. */
  ConstStringParameter *addConstString(const char *name, const char *str,
				      unsigned int roles=StreamOutput | Report);

  /* Add a string parameter of size N to this Menu. */
  template<int N>
  BaseStringParameter *addString(const char *name, const char *str,
				 unsigned int roles=SetValue | AllRoles);

  /* Add a string parameter of size N with selection to this Menu. */
  template<int N>
  BaseStringParameter *addString(const char *name, const char *str,
				 const char **selection, size_t n_selection=0,
				 unsigned int roles=SetValue | AllRoles);
  
  /* Add a boolean parameter to this Menu. */
  BoolParameter *addBoolean(const char *name, bool value,
			    unsigned int roles=SetValue | AllRoles);
  
  /* Add a integer parameter to this Menu. */
  NumberParameter<int> *addInteger(const char *name, int value,
				   const char *unit=0,
				   unsigned int roles=SetValue | AllRoles);
  
  /* Add a integer parameter to this Menu. */
  NumberParameter<int> *addInteger(const char *name, int value,
				   int minimum, int maximum,
				   const char *unit=0, const char *outunit=0,
				   unsigned int roles=SetValue | AllRoles);
  
  /* Add a float parameter to this Menu. */
  NumberParameter<float> *addFloat(const char *name, float value,
				   const char *format="%g", const char *unit=0,
				   unsigned int roles=SetValue | AllRoles);
  
  /* Add a float parameter to this Menu. */
  NumberParameter<float> *addFloat(const char *name, float value,
				   float minimum, float maximum,
				   const char *format="%g", const char *unit=0,
				   const char *outunit=0,
				   unsigned int roles=SetValue | AllRoles);

  /* Move action that was already added to this Menu to new position index. */
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

  /* Write name to stream. If descend, also display name and values
     of children. roles must be enabled. */
  virtual void write(Stream &stream=Serial, unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0, bool descend=true) const;

  /* Read configuration settings from instream as long as data are available
     or a line starting with "DONE" is encountered, and
     report errors on outstream. */
  virtual void read(Stream &instream=Serial, Stream &outstream=Serial);
  
  /* Interactive menu via serial stream.. */
  virtual void execute(Stream &stream=Serial);

  /* Set the provided name-value pair and report on stream. */
  virtual void set(const char *val, const char *name,
		   Stream &stream=Serial);


protected:

  static const size_t MaxActions = 16;
  size_t NActions;
  Action *Actions[MaxActions];
  bool Own[MaxActions];
  bool GoHome;
  
};


template<int N>
BaseStringParameter *Menu::addString(const char *name, const char *str,
				     unsigned int roles) {
  if ((name == 0) || (str == 0))
    return 0;
  StringParameter<N> *act = new StringParameter<N>(*this, name, str);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}


template<int N>
BaseStringParameter *Menu::addString(const char *name, const char *str,
				     const char **selection,
				     size_t n_selection, unsigned int roles) {
  if ((name == 0) || (str == 0))
    return 0;
  StringParameter<N> *act = new StringParameter<N>(*this, name, str,
						   selection, n_selection);
  Own[NActions - 1] = true;
  act->setRoles(roles);
  return act;
}


#endif
