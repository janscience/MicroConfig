![arduino-lint](https://github.com/janscience/MicroConfig/actions/workflows/arduino-lint.yml/badge.svg)

# MicroConfig

Configuration menus for microcontrollers.

Developed for [Teensy](https://www.pjrc.com/teensy/) and Arduino, but
should run on many more micro controllers, since it is mostly plain
C++ code.


## Features

- Interactive configuration and execution via serial streams.
- Store and retrieve configuration from EEPROM.
- Read and write YAML configuration file on SD card.
- Transfer configuration to and from host computer.
- Configures key-value pairs, with values being strings, enums, booleans, integer types, or floats.
- Numerical types with units and unit conversion.
- Two levels of access to configurable parameters: user and admin mode.
- Object-oriented and templated interface.
- Stores pointers to arbitarily sized action names (no memory consuming copies).
- Predefined menu for reporting, saving, loading and erasing configuration file on SD card as well as for putting and getting configuration from EEPROM.
- Predefined menu for uploading firmware, based on [FlasherX](https://github.com/joepasquariello/FlasherX).


## ToDo

- [ ] Add the general aspects of the [logger configuration
      GUI](https://github.com/janscience/TeeGrid/blob/main/utils/loggerconf.py)
      to the microconfig package.
- [ ] In unit conversion do not only check the prefix but also the unit itself.
- [ ] Support transfer of configuration to another micro controller via
      a serial stream.


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

Load the configuration from EEPROM and a file, and execute the menu like this:
```c
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  SD.begin(BUILTIN_SDCARD);      // initialize SD card
  config.get();                  // get configuration from EEPROM
  config.load();                 // load configuration file from SD card
  if (Serial)
    config.execute();            // execute the main menu, default 10s timeout
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
where you can choose to view the current configuration by hitting '1':

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

### GUI

As described above you can interact with the MicroConfig menu via the
serial stream yourself using, for example, the serial monitor of the
[Arduino IDE](https://docs.arduino.cc/software/ide/), or the
[serialmonitor](pymicroconfig/src/microconfig/serialmonitor.py)
provided by the [pymicroconfig](pymicroconfig/) package (also part of
the MicroConfig repository).

In addition, the [pymicroconfig](pymicroconfig/) package provides the
`microconfig` GUI for an even more comfortable interaction with the
microcontroller.

![mciroconfig GUI](pymicroconfig/docs/microconfig-menu.png)

See the [documentation](pymicroconfig/) of the pymicroconfig package
for a user guide and installation instructions.


## Interface

- [MicroConfig.h](src/MicroConfig.h): Include all headers of the MicroConfig library.
- [MicroConfigBanner](src/MicroConfigBanner.h): ASCII art banner and MicroConfig version for output streams.

### Basic menu structure

- [Action](src/Action.h): Base class for executable or configurable menu entries.
- [Parameter](src/Parameter.h): Actions with configurable name-value pairs of various types.
- [Menu](src/Menu.h): A menu of actions and parameters.
- [Config](src/Config.h): Root (top-level) Menu with configuration file.

### Message, help, and configuration menu

- [MessageAction.h](src/MessageAction.h): Action printing some text.
- [InfoAction.h](src/InfoAction.h): Action printing out key-value pairs.
- [HelpAction.h](src/HelpAction.h): Action printing a help message.
- [ConfigurationMenu.h](src/ConfigurationMenu.h): Actions and menu for managing configurations.

### Menu for firmware updates

- [FirmwareUpdate](src/FirmwareUpdate.h): Upload hex file from SD card.
- [FirmwareMenu.h](src/ConfigurationMenu.h): Actions and menu for handling firmware updates.


## Examples

In [examples/](examples) you find sketches demonstrating the use of
the MicroConfig libraries.

- [menu](examples/menu): Example demonstrating most features of the MicroConfig library.


## microconfig python package

The content of the [pymicroconfig/](pymicroconfig) folder is a python package.
It provides

- modules for detecting and interacting with MicroConfig menus on
  microcontrollers.
- [serialmonitor](pymicroconfig/src/microconfig/serialmonitor.py):
  serial monitor for the console that automatically detects Teensys
  connected to USB.
- [microconfig](pymicroconfig/src/microconfig/microconfig.py):
  GUI for setting configuration parameters on a microcontroller.

See [pymicroconfig/](pymicroconfig) for more details.


## Installation

There are three options to install the
[`MicroConfig`](https://github.com/janscience/MicroConfig) library:

- Install the
  [`MicroConfig`](https://github.com/janscience/MicroConfig) library
  from the library manager of the [Arduino
  IDE](https://docs.arduino.cc/software/ide/): Run the Arduino IDE,
  select the library manager from the icons on the left or from the
  menu (Tools-Manage libraries...). Type in `microconfig` at the top
  and make sure that `Type` and `Topic` is set to `All`. The
  MicroConfig library appears as the only search result. Simply hit
  the `INSTALL` button.

- Download the zip archive of the repository: Navigate to
  https://github.com/janscience/MicroConfig in your favorite browser.
  At the top right is a big, green `Code` button. Click it. A menu
  opens and at the bottom you find `Download ZIP`. Click it and the
  repository is downloaded into as zip file onto your computer.
  Extract the content of the zip file into the Arduino libraries
  folder.  After extraction, rename the folder `MicroConfig-main` to
  `MicroConfig`.

- Clone the respository into the Arduino libraries folder:
  ```
  cd Ardunio/libraries
  git clone https://github.com/janscience/MicroConfig.git
  ```
  This is recommended when you often need to update to the latest
  developments of the library and you do not want to wait for official
  releases. Once you cloned the library you can update it by pulling it
  ```
  cd Ardunio/libraries/MicroConfig
  git pull origin main
  ```

The Arduino libraries folder is located in
`/home/{username}/Arduino/libraries/` (Linux) or
`\Users\{username}\Documents\Arduino` (windows). If unsure, open the
[Arduino IDE](https://docs.arduino.cc/software/ide/) and select
`File-Preferences`. This shows you at the very top the sketchbook
location. Simply add `libraries` to this path.


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
