/*
  InfoAction - Action printing a text on stream.
  Created by Jan Benda, October 23rd, 2025.
*/

#ifndef InfoAction_h
#define InfoAction_h


#include <Action.h>


class InfoAction : public Action {

 public:

  /* Initialize and add to configuration menu.
     The text is not copied, only a pointer is stored.
     Make sure that the text is static. */
  InfoAction(Menu &menu, const char *name, const char *text);

  /* Print out text on stream. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);


 protected:

  const char *Text;
  
};


#endif
