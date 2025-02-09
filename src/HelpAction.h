/*
  HelpAction - Action printing a help message.
  Created by Jan Benda, February 9th, 2025.
*/

#ifndef HelpAction_h
#define HelpAction_h


#include <MicroConfig.h>


class HelpAction : public Action {

 public:

  /* Initialize and add to default menu. */
  HelpAction(const char *name);

  /* Initialize and add to menu. */
  HelpAction(Menu &menu, const char *name);

  /* Print help and key-bindings. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);
};


#endif
