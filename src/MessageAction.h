/*
  MessageAction - Action printing a text on stream.
  Created by Jan Benda, October 23rd, 2025.
*/

#ifndef MessageAction_h
#define MessageAction_h


#include <Action.h>


class MessageAction : public Action {

 public:

  /* Initialize and add to configuration menu.
     The text is not copied, only a pointer is stored.
     Make sure that the text is static. */
  MessageAction(Menu &menu, const char *name, const char *text=0);

  /* Set the text that is printed out by this action. */
  void setText(const char *text);

  /* Print out text on stream. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);


 protected:

  const char *Text;
  
};


#endif
