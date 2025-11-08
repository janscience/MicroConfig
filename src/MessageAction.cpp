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
			  size_t indent, size_t width, bool descend) const {
  if (disabled(roles))
    return;
  if (descend) {
    if (strlen(name()) > 0) {
      stream.printf("%*s%s:\n", indent, "", name());
      indent += indentation();
    }
    if (Text != 0)
      stream.printf("%*s%s:\n", indent, "", Text);
  }
  else
    Action::write(stream, roles, indent, width, descend);
}


void MessageAction::execute(Stream &stream, unsigned long timeout,
			    bool echo, bool detailed) {
  if (Text != 0)
    stream.println(Text);
  stream.println();
}
