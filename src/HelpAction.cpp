#include <HelpAction.h>


HelpAction::HelpAction(Menu &menu, const char *name) :
  Action(menu, name, StreamInput) {
}


void HelpAction::execute(Stream &stream, unsigned long timeout,
			 bool echo, bool detailed) {
  stream.println("- Select menu entries by entering the number followed by 'return'.");
  stream.println("- Go up to the parent menu by entering 'q'.");
  stream.println();
  stream.println("Special commands:");
  stream.println("- 'detailed on' : print additional infos as needed for GUIs, for example.");
  stream.println("- 'detailed off': do not print additional infos (default)");
  stream.println("- 'echo on'     : echo inputs (default)");
  stream.println("- 'echo off'    : do not echo inputs");
  stream.println("- 'print'       : print menu again");
  stream.println("- 'reboot'      : reboot");
  stream.println();
}

