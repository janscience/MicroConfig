#include <MessageAction.h>


MessageAction::MessageAction(Menu &menu, const char *name, const char *text, int roles) :
  Action(menu, name, roles),
  Text(text) {
}


MessageAction::MessageAction(Menu &menu, const char *name, int roles) :
  Action(menu, name, roles),
  Text(0) {
}


void MessageAction::setText(const char *text) {
  Text = text;
}


void MessageAction::execute(Stream &stream, unsigned long timeout,
			    bool echo, bool detailed) {
  if (Text != 0)
    stream.println(Text);
  stream.println();
}
