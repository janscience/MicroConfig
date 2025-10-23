#include <InfoAction.h>


InfoAction::InfoAction(Menu &menu, const char *name, const char *text) :
  Action(menu, name, StreamInput),
  Text(text) {
}


void InfoAction::setText(const char *text) {
  Text = text;
}


void InfoAction::execute(Stream &stream, unsigned long timeout,
			 bool echo, bool detailed) {
  if (Text != 0)
    stream.println(Text);
  stream.println();
}
