#include <HelpAction.h>


const char *help_text = R"HLP(- Select menu entries by entering the number followed by 'return'.
- Go up to the parent menu by entering 'q'.
- Go home to the top-level menu by entering 'h'.

Special commands:
- 'detailed on' : print additional infos as needed for GUIs, for example.
- 'detailed off': do not print additional infos (default)
- 'echo on'     : echo inputs (default)
- 'echo off'    : do not echo inputs
- 'gui on'      : inform the menu that a GUI is operating it
- 'gui off'     : manual interaction with the menu (default)
- 'show'        : show current menu settings
- 'print'       : print menu again
- 'reboot'      : reboot)HLP";


HelpAction::HelpAction(Menu &menu, const char *name) :
  MessageAction(menu, name, help_text) {
}
