#include <SD.h>
#include <Menu.h>
#include <Action.h>


bool Action::yesno(const char *request, bool defval,
		   bool echo, Stream &stream) {
  while (true) {
    stream.print(request);
    if (defval)
      stream.print(" [Y/n] ");
    else
      stream.print(" [y/N] ");
    while (stream.available() == 0)
      yield();
    char pval[8];
    stream.readBytesUntil('\n', pval, 8);
    if (echo)
      stream.println(pval);
    if (strlen(pval) == 0)
      return defval;
    if (tolower(pval[0]) == 'y')
      return true;
    if (tolower(pval[0]) == 'n')
      return false;
  }
}


Action::Action(const char *name, int roles) :
  Name(const_cast<char *>(name)),
  SupportedRoles(roles),
  Roles(roles),
  Indentation(2),
  Parent(NULL) {
}


Action::Action(Menu &menu, const char *name, int roles) :
  Action(name, roles) {
  menu.add(this);
}


void Action::setName(const char *name) {
  Name = new char[strlen(name) + 1];
  strcpy(Name, name);
}


void Action::clearName() {
  delete[] Name;
  Name = 0;
}


const Menu *Action::root() const {
  const Action *act = this;
  while (act->parent() != NULL)
    act = act->parent();
  return static_cast<const Menu*>(act);
}


Menu *Action::root() {
  Action *act = this;
  while (act->parent() != NULL)
    act = act->parent();
  return static_cast<Menu*>(act);
}


Action *Action::action(const char *name) {
  char lname[strlen(name) + 1];
  for (size_t k=0; k<strlen(name)+1; k++)
    lname[k] = tolower(name[k]);
  char cname[strlen(Name) + 1];
  for (size_t k=0; k<strlen(Name)+1; k++)
    cname[k] = tolower(Name[k]);
  if (strcmp(cname, lname) == 0)
    return this;
  return NULL;
}


bool Action::enabled(int roles) const {
  roles &= SupportedRoles;
  if (roles == 0)
    return false;
  return ((Roles & roles) > 0);
}


bool Action::disabled(int roles) const {
  return !enabled(roles);
}


void Action::enable(int roles) {
  roles &= SupportedRoles;
  Roles |= roles;
}


void Action::disable(int roles) {
  roles &= SupportedRoles;
  Roles &= ~roles;
}


void Action::disableSupported(int roles) {
  roles &= SupportedRoles;
  SupportedRoles &= ~roles;
  Roles = SupportedRoles;
}


void Action::report(Stream &stream, size_t indent,
		    size_t w, bool descend) const {
  if (enabled(StreamOutput) || enabled(StreamInput))
    stream.printf("%*s%s\n", indent, "", name());
}


void Action::save(File &file, int roles,
		  size_t indent, size_t w) const {
}


void Action::execute(Stream &stream, unsigned long timeout,
		     bool echo, bool detailed) {
}
