/*
  Storage - Interface to linear storage memory, like EEPROM.
  Created by Jan Benda, August 14, 2026.

  The Storage class is used to access linear storage memories, like EEPROM,
  for storing a configuration.
  
  This default implementation provides access to the internal EEPROM
  as provided by the global EEPROM variable. An instance is made available
  as EEPROMStorage.

  When reimplementing this class for another EEPROM memory, for example,
  one that is accesible via I2C bus, then reimplement the length(),
  read(), and update() functions.
*/

#ifndef Storage_h
#define Storage_h


class Storage {

 public:

  // Constructor using internal EEPROM.
  Storage();

  // Size of storage in bytes.
  virtual uint16_t length();

  // Read variable t at index idx.
  template<typename T>
    T &get(int idx, T &t);
  
  // Write variable t to index idx.
  template<typename T>
    const T &put(int idx, const T &t);

  // Compute CRC sum.
  uint32_t crc(int addr0, int addr1);


protected:

  // Read len bytes from storage at idx into buffer at address dest.
  virtual void read(int idx, uint8_t *dest, size_t len);
  
  // Write a len bytes from buffer at address src to storage at idx.
  virtual void update(int idx, const uint8_t *src, size_t len);

};


static Storage EEPROMStorage __attribute__ ((unused));


template<typename T>
T &Storage::get(int idx, T &t) {
  read(idx, (uint8_t *) &t, sizeof(T));
  return t;
}


template<typename T>
const T &Storage::put(int idx, const T &t) {
  update(idx, (const uint8_t *) &t, sizeof(T));
  return t;
}

#endif
