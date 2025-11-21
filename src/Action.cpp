#include <Menu.h>
#include <Config.h>
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


Action::Action(const char *name, unsigned int roles) :
  Name(const_cast<char *>(name)),
  SupportedRoles(roles),
  Roles(roles),
  Parent(NULL),
  Root(NULL) {
}


Action::Action(Menu &menu, const char *name, unsigned int roles) :
  Action(name, roles) {
  menu.add(this);
}


Action::~Action() {
}


void Action::setName(const char *name) {
  Name = new char[strlen(name) + 1];
  strcpy(Name, name);
}


void Action::clearName() {
  delete[] Name;
  Name = 0;
}


void Action::setRoot(Config *root) {
  Root = root;
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


bool Action::enabled(unsigned int roles) const {
  roles &= SupportedRoles;
  if (roles == 0)
    return false;
  return ((Roles & roles) > 0);
}


bool Action::disabled(unsigned int roles) const {
  return !enabled(roles);
}


void Action::enable(unsigned int roles) {
  roles &= SupportedRoles;
  Roles |= roles;
}


void Action::disable(unsigned int roles) {
  roles &= SupportedRoles;
  Roles &= ~roles;
}


void Action::disableSupported(unsigned int roles) {
  roles &= SupportedRoles;
  SupportedRoles &= ~roles;
  Roles &= SupportedRoles;
}


void Action::setRoles(unsigned int roles) {
  SupportedRoles = roles;
  Roles = roles;
}


size_t Action::indentation() const {
  return Root == 0 ? 4 : Root->Indentation;
}


bool Action::echo() const {
  return Root == 0 ? true : Root->Echo;
}


bool Action::detailed() const {
  return Root == 0 ? false : Root->Detailed;
}


bool Action::gui() const {
  return Root == 0 ? false : Root->GUI;
}


void Action::write(Stream &stream, unsigned int roles, size_t indent,
		   size_t width, bool descend) const {
  if (enabled(roles))
    stream.printf("%*s%s\n", indent, "", name());
}


void Action::execute(Stream &stream) {
  write(stream);
  stream.println();
}


int Action::put(int addr, int &num, Stream &stream) const {
  return addr;
}


int Action::get(int addr, int &num, bool setvalue, Stream &stream) {
  return addr;
}

