/*
  HelpAction - Action printing a help message.
  Created by Jan Benda, February 9th, 2025.
*/

#ifndef HelpAction_h
#define HelpAction_h


#include <MessageAction.h>


class HelpAction : public MessageAction {

 public:

  /* Initialize and add to menu. */
  HelpAction(Menu &menu, const char *name);
  
};


#endif
