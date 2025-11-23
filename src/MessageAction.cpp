#include <MessageAction.h>


MessageAction::MessageAction(Menu &menu, const char *name,
			     const char *text, unsigned int roles) :
  Action(menu, name, roles),
  Text(text) {
}


MessageAction::MessageAction(Menu &menu, const char *name,
			     unsigned int roles) :
  Action(menu, name, roles),
  Text(0) {
}


void MessageAction::setText(const char *text) {
  Text = text;
}


void MessageAction::write(Stream &stream, unsigned int roles,
			  size_t indent, size_t width) const {
  if (name() != 0 && strlen(name()) > 0) {
    stream.printf("%*s%s:\n", indent, "", name());
    indent += indentation();
  }
  if (Text != 0)
    stream.printf("%*s%s:\n", indent, "", Text);
}


void MessageAction::execute(Stream &stream) {
  if (Text != 0)
    stream.println(Text);
  stream.println();
}
