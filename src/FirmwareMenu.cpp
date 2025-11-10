#include <FirmwareUpdate.h>
#include <FirmwareMenu.h>


void ListFirmwareAction::execute(Stream &stream) {
  if (disabled(StreamInput))
    return;
  listFirmware(SDC, stream);
  stream.println();
}


void UpdateFirmwareAction::execute(Stream &stream) {
  if (disabled(StreamInput))
    return;
  updateFirmware(SDC, echo(), detailed(), stream);
  stream.println();
}


FirmwareMenu::FirmwareMenu(Menu &menu, SDClass &sdcard) :
  Menu(menu, "Firmware", Action::StreamInput),
  ListAct(*this, "List available updates", sdcard),
  UpdateAct(*this, "Update firmware", sdcard) {
}

