#include <EEPROM.h>
#include <Storage.h>


Storage::Storage() {
}


uint16_t Storage::length() {
  return EEPROM.length();
}


int Storage::read(unsigned int idx, uint8_t *dest, size_t len) {
  size_t n = len;
  while (len--)
    *dest++ = EEPROM.read(idx++);
  return n;
}

  
int Storage::update(unsigned int idx, const uint8_t *src, size_t len) {
  size_t n = len;
  while (len--)
    EEPROM.update(idx++, *src++);
  return n;
}


uint32_t Storage::crc(int addr0, int addr1) {
  // Adapted from https://docs.arduino.cc/learn/programming/eeprom-guide/
  
  const uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  if (addr0 < 0)
    addr0 = 0;
  if (addr1 > length())
    addr1 = length();
  uint8_t buffer[addr1 - addr0];
  read(addr0, buffer, addr1 - addr0);
  uint8_t val = 0;
  uint32_t crc = ~0L;
  for (int index = 0; index < addr1 - addr0; ++index) {
    val = buffer[index];
    crc = crc_table[(crc ^ val) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (val >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

