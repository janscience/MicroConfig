/*
  Storage - Interface to linear storage memory, like EEPROM.
  Created by Jan Benda, August 14, 2026.
*/

#ifndef Storage_h
#define Storage_h


#include <EEPROM.h>


class Storage {

 public:

  // Constructor using internal EEPROM.
  Storage();

  // Size of the storage in bytes.
  uint16_t length();

  // Read variable t at index idx.
  template<typename T>
    T &get(int idx, T &t);
  
  // Write variable t to index idx.
  template<typename T>
    const T &put(int idx, const T &t);

  // Compute CRC sum.
  uint32_t crc(int addr0, int addr1);

};


template<typename T>
T &Storage::get(int idx, T &t) {
  return EEPROM.get(idx, t);
}


template<typename T>
const T &Storage::put(int idx, const T &t) {
  return EEPROM.put(idx, t);
}

#endif
