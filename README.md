![arduino-lint](https://github.com/janscience/MicroConfig/actions/workflows/arduino-lint.yml/badge.svg)

# MicroConfig

Configuration menus for microcontrollers.

Developed for [Teensy](https://www.pjrc.com/teensy/) and Arduino, but
should run on many more micro controllers, since it is mostly plain
C++ code.


## Features

- Interactive configuration and execution via serial streams.
- Read and write YAML configuration file on SD card.
- Configures key-value pairs, with values being strings, enums, booleans, integer types, or floats.
- Numerical types with units and unit conversion.
- Object-oriented and templated interface.
- Stores pointers to arbitarily sized action names (no memory consuming copies).
- Predefined menu for reporting, saving, loading and erasing configuration file.
- Predefined menu for uploading firmware, based on [FlasherX](https://github.com/joepasquariello/FlasherX).


## ToDo

- [ ] Make the [serialmonitor.py](utils/serialmonitor.py) and the
      general aspects of the [logger configuration
      GUI](https://github.com/janscience/TeeGrid/blob/main/utils/loggerconf.py)
      a proper python package.
- [ ] In unit conversion do not only check the prefix but also the unit itself.
- [ ] Support transfer of configuration to another micro controller via
      a serial stream.
- [ ] Store and retrieve configuration to EEPROM in a compact way.


## Usage

This is a brief teaser, for details see the [user guide](docs/userguide.md).

Define a menu with a number of submenus and configurable parameters:

```c
#include <SD.h>
#include <MicroConfig.h>

Config config("micro.cfg", &SD);               // main menu and configuration file
Menu settings(config, "Settings");             // settings sub menu
StringParameter<32> path(settings, "Path", "recordings/");
char filename[64] = "recording.wav";
StringPointerParameter<64> file_name(settings, "Recording", &filename);
NumberParameter<float> file_time(settings, "FileTime", 30.0, 1.0, 8640.0, "%.0f", "s");

Menu aisettings(config, "Analog input");       // analog input sub menu
NumberParameter<uint32_t> rate(aisettings, "SamplingRate", 48000, 1, 1000000, "%.1f", "Hz", "kHz");

// sub menu for reporting, saving, loading and removing configuration file:
ConfigurationMenu configuration_menu(config, SD);
HelpAction help_act(config, "Help");
```

Load the configuration file and execute the menu like this:
```c
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  SD.begin(BUILTIN_SDCARD);              // initialize SD card
  config.load();                         // load configuration file from SD card
  if (Serial)
    config.execute(Serial, 10000);       // execute the main menu, 10s timeout
}
```

Then you get on the serial monitor:

```txt
Configuration file "micro.cfg" not found or empty.

Menu:
  1) Settings ...
  2) Analog input ...
  3) Configuration ...
  4) Help
  Select: 
```

The first two sub menus allow you to change the values of the
parameters defined above.

Hit '3' followed by return to enter the configuration menu,
where you can choose to view the current configuration:

```txt
Settings:
  Path:     recordings/
  FileTime: 300s
Analog input:
  SamplingRate: 48.0kHz
```

The configuration file, that you can also save from this menu, looks
exactly like this as well.  The format is compatible with YAML files
and can be easily edited.

The configuration values can be accessed by the `.value()` member
functions. The value of the file name parameter is directly stored in
the provided variable. The following code

```c
  Serial.printf("path: %s\n", path.value());
  Serial.printf("file name: %s\n", filename);
  Serial.printf("file time: %g\n", file_time.value());
  Serial.printf("sampling rate: %u\n", rate.value());
```

prints

```txt
path: recordings/
file name: recording.wav
file time: 30
sampling rate: 48000
```

Note that for the file name we do not need to retrieve the value from
the parameter. Instead we just use the variable that has been
automagically set by the StringPointerParameter.

Nice and easy, isn't it?


## Interface

- [MicroConfig.h](src/MicroConfig.h): Include all headers of the MicroConfig library.
- [MicroConfigBanner](src/MicroConfigBanner.h): ASCII art banner and MicroConfig version for output streams.

### Basic menu structure

- [Action](src/Action.h): Base class for executable or configurable menu entries.
- [Parameter](src/Parameter.h): Actions with configurable name-value pairs of various types.
- [Menu](src/Menu.h): A menu of actions and parameters.
- [Config](src/Config.h): Menu with configuration file.

### Message, help, and configuration menu

- [MessageAction.h](src/MessageAction.h): Action printing some text.
- [HelpAction.h](src/HelpAction.h): Action printing a help message.
- [ConfigurationMenu.h](src/ConfigurationMenu.h): Actions and menu for managing configurations.

### Menu for firmware updates

- [FirmwareUpdate](src/FirmwareUpdate.h): Upload hex file from SD card.
- [FirmwareMenu.h](src/ConfigurationMenu.h): Actions and menu for handling firmware updates.


## Examples

In [examples/](examples) you find sketches demonstrating the use of
the MicroConfig libraries.

- [menu](examples/menu): Example demonstrating most features of the MicroConfig library.


## Utilities

In [utils/](utils) you find useful python scripts:

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
