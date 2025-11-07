#include <FirmwareUpdate.h>
#include <FirmwareMenu.h>


void ListFirmwareAction::execute(Stream &instream, Stream &outstream,
				 unsigned long timeout, bool echo,
				 bool detailed) {
  if (disabled(StreamInput))
    return;
  listFirmware(SDC, outstream);
  outstream.println();
}


void UpdateFirmwareAction::execute(Stream &instream, Stream &outstream,
				   unsigned long timeout, bool echo,
				   bool detailed) {
  if (disabled(StreamInput))
    return;
  updateFirmware(SDC, echo, detailed, instream, outstream);
  outstream.println();
}


FirmwareMenu::FirmwareMenu(Menu &menu, SDClass &sdcard) :
  Menu(menu, "Firmware", Action::StreamInput),
  ListAct(*this, "List available updates", sdcard),
  UpdateAct(*this, "Update firmware", sdcard) {
}

