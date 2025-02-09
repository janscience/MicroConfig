![arduino-lint](https://github.com/janscience/MicroControl/actions/workflows/arduino-lint.yml/badge.svg)

# MicroConfig

Configuration menus for microcontrollers.


## Features

- Interactive configuration and execution via serial streams.
- Read and write YAML configuration file on SD card.
- Configures key-value pairs, with values being strings, enums, integer types, floats.
- Numerical types with units and unit conversion.
- Object-oriented and templated interface.
- Predefined menu for reporting, saving, loading and erasing configuration file.
- Predefined menu for uploading firmware.


## Libraries

- [Action](src/Action.h): Base class for executable or configurable menu entries.
- [Parameter](src/Parameter.h): Actions with configurable name-value pairs of various types.
- [Configurable](src/Configurable.h): A collection of Actions and Parameters.
- [Configurator](src/Configuration.h): Configure configurable class instances from a configuration file.
- [FirmwareUpdate](src/FirmwareUpdate.h): Upload hex file from SD card.
- [MicroConfigBanner](src/MicroConfigBanner.h): ASCII art banner and MicroConfig version for output streams.


## Examples

In [examples/](examples) you find sketches demonstrating the use of
the MicroConfig libraries.

- [writeconfig](examples/writeconfig): Write a default configuration file on SD card.


## Utilities

In [utils/](utils) you find some useful python scripts:

- [serialmonitor](utils/serialmonitor.py): serial monitor for the
  console that automatically detects Teensys connected to USB.


## Applications of the MicroConfig libraries

MicroConfig is used in:

- [TeeRec](https://github.com/janscience/TeeRec): Libraries for
  recording analog input data on
  [Teensy](https://www.pjrc.com/teensy/) microcontrollers.

- [TeeGrid](https://github.com/janscience/TeeGrid): Electrode arrays
  based on 8-16channel recording devices for recording electric fish
  behavior in natural habitats (see [Henninger et
  al. 2018](https://doi.org/10.1523/JNEUROSCI.0350-18.2018) and
  [Henninger et al. 2020](https://doi.org/10.1242/jeb.206342)).

- [FishFinder](https://github.com/janscience/FishFinder): Smart
  fishfinders for better EOD recordings of electric fish in the field.
