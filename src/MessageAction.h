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
  MessageAction(Menu &menu, const char *name, const char *text, unsigned int roles=StreamInput);

  /* Initialize and add to configuration menu.
     You need to assign a text later on using setText(). */
  MessageAction(Menu &menu, const char *name, unsigned int roles=StreamInput);

  /* Set the text that is printed out by this action.
     The text is not copied, only a pointer is stored.
     Make sure that the text is static. */
  void setText(const char *text);

  /* Write the actions's name and the text to stream.
     roles must be enabled. */
  virtual void write(Stream &stream, unsigned int roles=AllRoles,
		     size_t indent=0, size_t width=0, bool descend=true) const;

  /* Print out text on stream. */
  virtual void execute(Stream &instream=Serial, Stream &outstream=Serial,
		       unsigned long timeout=0, bool echo=true,
		       bool detailed=false);


 protected:

  const char *Text;
  
};


#endif
