![arduino-lint](https://github.com/janscience/MicroControl/actions/workflows/arduino-lint.yml/badge.svg)

# MicroConfig

Configuration menus for microcontrollers.

Developed for [Teensy](https://www.pjrc.com/teensy/) and Arduino, but
should run on many more micro controllers, since it is mostly plain
C++ code.


## Features

- Interactive configuration and execution via serial streams.
- Read and write YAML configuration file on SD card.
- Configures key-value pairs, with values being strings, enums, integer types, or floats.
- Numerical types with units and unit conversion.
- Object-oriented and templated interface.
- Predefined menu for reporting, saving, loading and erasing configuration file.
- Predefined menu for uploading firmware.


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

The following example code is essentially the [menu](examples/menu/menu.ino)
example.

First, include the `M̀icroConfig.h` header and define the main menu
`config`.  Also include `SD.h` needed for handling the configuration
file on the SD card:

```c
#include <SD.h>
#include <MicroConfig.h>

Configurator config;
```

Next we add a menu with name "Settings" to the main menu.  This menu
gets two parameters, one a string type defining a path on the SD
card where to store recording files, and one a floating point number
defining the duration of a recording. All parameters take as first
arguments the menu where they are added to, their name, and their
value in the respective type. Further arguments provide additional
properties of the parameters. In particular, numerical types take
minimum and maximum values, a format string, and a unit:

```c
Menu settings("Settings");                 // settings menu
StringParameter<64> Path(                     // string parameter with max 64 characters
		         settings,            // add it to settings menu
                         "Path",              // name
                         "recordings/");      // value
NumberParameter<float> FileTime(              // float parameter
                                settings,     // add it to settings menu
                                "FileTime",   // name
                                30.0,         // value
                                1.0,          // minimum valid value
                                8640.0,       // maximum valid value
                                "%.0f",       // format string
                                "s");         // unit of the value
```

We add another menu with name "Analog input". This menu gets an integer
type parameter that internally stores its values in "Hz", but uses
"kHz" when interacting with the user. The user may enter values to
this parameter also in other units, like for example, "mHz", "MHz" or
"GHz". All these inputs are then converted to "Hz".

```c
Menu aisettings("Analog input");           // analog input menu
NumberParameter<uint32_t> Rate(               // unit32_t parameter
                               aisettings,    // add it to aisettings menu
			       "SamplingRate",// name 
			       48000,         // value (in Hz)
			       1,             // minimum valid value (in Hz)
			       1000000,       // maximum valid value (in Hz)
			       "%.1f",        // format string (for kHz)
			       "Hz",          // unit of the internal value
			       "kHz");        // use this unit in user interactions
```

Finally, let's add the predefined menus for handling the configuration
file, firmware updates, and printing a help message:

```c
ConfigurationMenu configuration_menu(SD);  // interactively report, save, load and remove configuration file
FirmwareMenu firmware_menu(SD);            // menu for uploading hex files from SD card
HelpAction help_act(config, "Help");       // action showing how to use the menu
```

The main code initializes the Serial stream and the builtin SD card,
tells the main menu the name of the configuration file to be used,
loads the configuration file from SD card (if available), and executes
the main menu.

```c
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  printMicroConfigBanner();                // print a nice banner
  SD.begin(BUILTIN_SDCARD);                // initialize SD card
  config.setConfigFile("micro.cfg");       // use "micro.cfg" as configuration file 
  config.load(SD);                         // load configuration file from SD card
  if (Serial)
    config.execute(Serial, 10000);         // execute the main menu, 10s timeout
  config.report();                         // report the parameter settings
}


void loop() {
}

```

Compile and upload this sketch. Then watch the output with the serial
monitor of the Arduino IDE or with the provided
[serialmonitor.py](utils/serialmonitor.py) python script. You get this:

```txt
========================================================
 __  __ _                 ____             __ _       
|  \/  (_) ___ _ __ ___  / ___|___  _ __  / _(_) __ _ 
| |\/| | |/ __| '__/ _ \| |   / _ \| '_ \| |_| |/ _` |
| |  | | | (__| | | (_) | |__| (_) | | | |  _| | (_| |
|_|  |_|_|\___|_|  \___/ \____\___/|_| |_|_| |_|\__, |
                                                |___/ 

version 0.3.0 by Benda-Lab
--------------------------------------------------------

Configuration file "micro.cfg" not found or empty.

Menu:
  1) Settings ...
  2) Analog input ...
  3) Configuration ...
  4) Firmware ...
  5) Help
  Select: 
```

Nice banner, right? And a nicely formatted menu. `...` indicates a submenu.

If there is no user input over the serial stream for more than 10s,
then the menu is aborted and the program continues.

So, type in `1` followed by return quickly enough to enter the
"Settings" menu:

```txt
Settings:
  1) Path: recordings/
  2) FileTime: 30s
  Select [1]:
```

To change the value of "FileTime" enter `2`:

```txt
FileTime        : 30s
Enter new value  (between 1s and 8640s): 
```

Numerical parameter support unit conversion, the user may specify a
new value in a different unit. Here, the time can be also specified in
"min" or "h", for example. The entered value is then converted to
"s". Without specifying a unit, the default "s" is assumed.  We enter
`5min`, and the menu correctly reports `300s`:

```txt
Settings:
  1) Path: recordings/
  2) FileTime: 300s
  Select [2]: 
```

The `[2]` in square brackets indicates the default input when you just
hit enter. It is set to the menu entry that you entered previously.

`q` brings you up one level, that is back to the main menu. There,
enter `3` to enter the configuration menu:

```txt
Configuration:
  1) Print configuration
  2) Save configuration
  3) Load configuration
  4) Erase configuration
  Select [1]: 
```

Enter `1` to print the current configuration:

```txt
Settings:
  Path:     recordings/
  FileTime: 300s
Analog input:
  SamplingRate: 48.0kHz
```

`2` saves the current configuration to the SD card. The content of the
configuration file looks exactly like the one we just have
printed. This output is compatible with YAML files, if you care. But
more importantly, this output format is well readable by humans, and
can be easily edited in any text editor. If there is already a
configuration file, you get a confirmation message like this:

```txt
Configuration file "micro.cfg" already exists on SD card.
Do you want to overwrite the configuration file? [Y/n]
```

Hit `ỳ` or `enter` (the capital `Y` indicates the default) to
overwrite the existing configuration file.

Nice and easy, isn't it?


## Interface

- [MicroConfig.h](src/MicroConfig.h): Include all headers of the MicroConfig library.
- [MicroConfigBanner](src/MicroConfigBanner.h): ASCII art banner and MicroConfig version for output streams.

### Basic menu structure

- [Action](src/Action.h): Base class for executable or configurable menu entries.
- [Parameter](src/Parameter.h): Actions with configurable name-value pairs of various types.
- [Menu](src/Menu.h): A menu of actions and parameters.
- [Configurator](src/Configuration.h): Configure menus from a configuration file.

### Help and configuration menu

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
