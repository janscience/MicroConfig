#include <MessageAction.h>


MessageAction::MessageAction(Menu &menu, const char *name, const char *text) :
  Action(menu, name, StreamInput),
  Text(text) {
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
