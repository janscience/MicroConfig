#include <FirmwareUpdate.h>
#include <FirmwareMenu.h>


void ListFirmwareAction::execute(Stream &stream, unsigned long timeout,
				 bool echo, bool detailed) {
  if (disabled(StreamInput))
    return;
  listFirmware(SDC, stream);
  stream.println();
}


void UpdateFirmwareAction::execute(Stream &stream, unsigned long timeout,
				   bool echo, bool detailed) {
  if (disabled(StreamInput))
    return;
  updateFirmware(SDC, echo, stream);
  stream.println();
}


FirmwareMenu::FirmwareMenu(Menu &menu, SDClass &sdcard) :
  Menu(menu, "Firmware", Action::StreamInput),
  ListAct(*this, "List available updates", sdcard),
  UpdateAct(*this, "Update firmware", sdcard) {
}

