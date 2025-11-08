#include <SD.h>
#include <InfoAction.h>


InfoAction::InfoAction(Menu &menu, const char *name, unsigned int roles) :
  Action(menu, name, roles),
  NKeyVals(0),
  MaxWidth(0) {
}


InfoAction::InfoAction(Menu &menu, const char *name,
		       const char *key1, const char *value1,
		       const char *key2, const char *value2,
		       const char *key3, const char *value3,
		       const char *key4, const char *value4,
		       const char *key5, const char *value5,
		       const char *key6, const char *value6) :
  Action(menu, name, StreamIO | Report),
  NKeyVals(0) {
  add(key1, value1);
  add(key2, value2);
  add(key3, value3);
  add(key4, value4);
  add(key5, value5);
  add(key6, value6);
}


int InfoAction::add(const char *key, const char *value) {
  if ((key == 0) || (value == 0))
    return -1;
  int i = setValue(key, value);
  if (i >= 0)
    return i;
  if (NKeyVals >= MaxKeyVals)
    return -1;
  Keys[NKeyVals] = key;
  Values[NKeyVals] = value;
  if (strlen(key) > MaxWidth)
    MaxWidth = strlen(key);
  return ++NKeyVals;
}


bool InfoAction::setValue(size_t index, const char *value) {
  if (index >= NKeyVals)
    return false;
  if (value == 0)
    return false;
  Values[index] = value;
  return true;
}


int InfoAction::setValue(const char *key, const char *value) {
  for (size_t k=0; k<NKeyVals; k++) {
    if (strcmp(Keys[k], key) == 0) {
      if (setValue(k, value))
	return k;
      else
	return -1;
    }
  }
  return -1;
}


void InfoAction::write(Stream &stream, unsigned int roles,
		       size_t indent, size_t width, bool descend) const {
  if (disabled(roles))
    return;
  if (descend) {
    if (strlen(name()) > 0) {
      stream.printf("%*s%s:\n", indent, "", name());
      indent += indentation();
      width = MaxWidth;
    }
    else if (width < MaxWidth)
      width = MaxWidth;
    for (size_t k=0; k<NKeyVals; k++) {
      size_t kw = width >= strlen(Keys[k]) ? width - strlen(Keys[k]) : 0;
      stream.printf("%*s%s:%*s %s\n", indent, "", Keys[k], kw, "", Values[k]);
    }
  }
  else if (strlen(name()) > 0)
    Action::write(stream, roles, indent, width, descend);
}


void InfoAction::execute(Stream &stream, unsigned long timeout,
			 bool echo, bool detailed) {
  write(stream, AllRoles, 0, MaxWidth, true);
  stream.println();
}
