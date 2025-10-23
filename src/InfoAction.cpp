#include <InfoAction.h>


InfoAction::InfoAction(Menu &menu, const char *name, const char *text) :
  Action(menu, name, StreamInput),
  Text(text) {
}


void InfoAction::execute(Stream &stream, unsigned long timeout,
			 bool echo, bool detailed) {
  stream.println(Text);
  stream.println();
}
