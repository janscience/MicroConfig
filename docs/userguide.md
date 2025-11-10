# User guide

The following example code mostly follows the [menu](examples/menu/menu.ino)
example.

You need to include the `M̀icroConfig.h` header.  Also include `SD.h`
needed for handling the configuration file on the SD card:

```c
#include <SD.h>
#include <MicroConfig.h>
```

## Content

- [Main menu](#main-menu)
- [Sub menus](#sub-menus)
- [Parameter](#parameter)
- [Actions](#actions)
- [Banner](#banner)


## Main menu

First, define the main menu `config` with configuration file
"micro.cfg" on default SD card:

```c
Config config("micro.cfg", &SD);
```

In `setup()` we initialize the serial interface and the SD card
and then execute the configuration menu:

```c
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  SD.begin(BUILTIN_SDCARD);   // initialize SD card
  if (Serial)
    config.execute();         // execute the main menu, default 10s timeout
```


## Sub menus

Next we add two sub menus with names "Settings" and "Analog input" to
the main menu. Later we will add a few configurable parameter to these menus.

```c
Menu settings(config, "Settings");        // settings menu
Menu aisettings(config, "Analog input");  // analog input menu
```

Let's also add the predefined menus for handling the configuration
file, firmware updates, and printing a help message:

```c
ConfigurationMenu configuration_menu(config, SD);  // interactively report, save, load and remove configuration file
FirmwareMenu firmware_menu(config, SD);            // menu for uploading hex files from SD card
HelpAction help_act(config, "Help");               // action showing how to use the menu
```

When executing this menu, you see in your serial monitor:

```txt
Menu:
  1) Settings ...
  2) Analog input ...
  3) Configuration ...
  4) Firmware ...
  5) Help
  Select: 
```

Sub menus are indicated by `...`.  You navigate this menu by entering
a number followed by return. `q` brings you up one level and also
quits the menu. `h` always brings you back to the main menu.

If there is no user input over the serial stream for more than 10s,
then the menu times out and the program continues.

So, enter `5` quickly enough to trigger the "Help" action. This simply
prints out what we just said and in addition some special commands:

```txt
- Select menu entries by entering the number followed by 'return'.
- Go up to the parent menu by entering 'q'.
- Go home to the top-level menu by entering 'h'.

Special commands:
- 'detailed on' : print additional infos as needed for GUIs, for example.
- 'detailed off': do not print additional infos (default)
- 'echo on'     : echo inputs (default)
- 'echo off'    : do not echo inputs
- 'print'       : print menu again
- 'reboot'      : reboot

```

Try them out! The effect of `detailed on`, however, affects mostly
parameters.

Let us now populate the sub menus with some parameter.


## Parameter

Parameter allow you to configure strings, enums, booleans, integer types, or
floats by means of template classes.

All parameters take as first arguments the menu where they are added
to, their name, and their value in the respective type.

In the following we demonstrate how to define a few paraemters of
different types.


### String parameter

The first parameter is a simple string that allows you to configure a
path on the SD card where to store files. The template parameter, here
32, sets the number of maximum characters the string can hold:

```c
StringParameter<32> path(settings,        // add it to settings menu
                         "Path",          // name
                         "recordings/");  // value
```

This parameter owns its value, which can be retrieved by the `path.value()`
member function as a char pointer.

Enter `1` to enter the settings menu:

```txt
Settings:
  1) Path: recordings/
  2) Recording: recording.wav
  3) FileTime: 300s
  Select [1]:
```

The `[1]` in square brackets indicates the default input when you just
hit enter. It is set to the menu entry that you entered last.

Enter `1` to edit the path variable, This displays the value of the
variable and asks for a new value:

```txt
Path            : recordings/
Enter new value :
```

Enter a new value, for example "data/" followed by return, and you get

```txt
Settings:
  1) Path: data/
  2) Recording: recording.wav
  3) FileTime: 300s
  Select [1]: 
```


### String pointer parameter

A parameter can also configure a variable directly. Such parameter
takes as a value a pointer to the variable. As an example, we define a
character array for a file name and pass it to a string pointer
parameter:

```c
char filename[64] = "recording.wav";
StringPointerParameter<64> file_name(settings, "Recording", &filename);
```

In the menu, the string pointer parameter is displayed and edited
exactly like the string parameter discussed above.

For all other parameter types, a pointer variant that takes as a value
a pointer to a variable of corrseponding type also exist.


### Float parameter with unit

We also want to add the duration of a recording to the settings sub
menu.  This is a number parameter with template parameter `float`.
Further arguments to a number parameter provide the minimum and
maximum allowed values, a printf-style format string used for
displaying the value, and a unit:

```c
NumberParameter<float> file_time(settings, "FileTime", 30.0,
                                 1.0,     // minimum valid value
                                 8640.0,  // maximum valid value
                                 "%.0f",  // format string
                                 "s");    // unit of the value
```

To change the value of "FileTime" enter `2` in the settings menu:

```txt
FileTime        : 30s
Enter new value  (between 1s and 8640s): 
```

The prompt reminds you about valid input values. If you enter an
invalid number, you are asked again to enter a new value. Hitting
enter without entering a number goes back to the settings menu and
keeps the value without changing it.

Numerical parameters support unit conversion, you may specify a new
value in a different unit. Here, the time can be also specified in
"min" or "h", for example. The entered value is then converted to
"s". Without specifying a unit, the default "s" is assumed.  We enter
`5min`, and the menu correctly reports `300s`:

```txt
Settings:
  1) Path: recordings/
  2) Recording: recording.wav
  3) FileTime: 300s
  Select [2]: 
```


### Integer parameter

The analog input menu gets an entry specifying the sampling
rate. Internaly the sampling rate is an unsigned integer given in
Hz. But with the user we want to communicate the sampling rate in kHz
(in the menu and in the configuration file). This is supported by the
number parameters irrespective of their type. Simply provide a second unit.

```c
NumberParameter<uint32_t> rate(aisettings, "SamplingRate",
                               48000,         // value (in Hz)
                               1,             // minimum valid value (in Hz)
                               1000000,       // maximum valid value (in Hz)
                               "%.1f",        // format string (for kHz)
                               "Hz",          // unit of the internal value
                               "kHz");        // use this unit in user interactions
```

When displayed in the menu, or when prompting for a new value, the
value is reported in this secondary unit:

```txt
SamplingRate    : 48.0kHz
Enter new value  (between 0.0kHz and 1000.0kHz): 
```

When entering a number without unit, it is interpreted in the unit
shown, here "kHz". Again, you may enter values to this parameter in
other units, like for example, "mHz", "MHz" or "GHz". All these inputs
are then converted internally to "Hz" and are displayed in
"kHz". Let's enter '0.5MHz" and we get the expected 500kHz:

```txt
Analog input:
  1) SamplingRate: 500.0kHz
  2) SamplingSpeed: medium
  Select [1]: 
```

When retrieving the value of this parameter via `rate.value()`, you
get an unsigned integer with value 500000 - the sampling rate in the
primary unit, here in Hz.


### Enum parameter

Let's also add an enum parameter to the analog input menu.
It allows to select values from an enum type. For this, 
an array with all the enum types and an array with
corresponding strings are required:

```c
// definition of enum type:
enum SAMPLING_SPEED {LOW_SPEED, MED_SPEED, HIGH_SPEED};
// array of all enum values:
SAMPLING_SPEED SamplingEnums[3] = {LOW_SPEED, MED_SPEED, HIGH_SPEED};
// corresponding string representations:
const char *SamplingStrings[3] = {"low", "medium", "high"};
EnumParameter<SAMPLING_SPEED> speed(aisettings, "SamplingSpeed", MED_SPEED,
                                    SamplingEnums,    // array of enums
                                    SamplingStrings,  // array of corresponding strings
                                    3);               // number of values in the arrays                               
```

With this enum parameter, the analog input menu looks like this:

```txt
Analog input:
  1) SamplingRate: 48.0kHz
  2) SamplingSpeed: medium
  Select [1]:
```

When entering `2` to select SamplingSpeed you get a selection of
possible enum values:

```txt
SamplingSpeed   : medium
  - 1) low
  - 2) medium
  - 3) high
Select new value:
```

Type in `3` to select "high". And indeed, you get

```txt
Analog input:
  1) SamplingRate: 48.0kHz
  2) SamplingSpeed: high
  Select [2]: 
```

Retrieving the value via `speed.value()` returns the string representation,
since the `EnumParameter` class is derived from the `BaseStringParameter`
class. If you want the integer enum value us `speed.enumValue()`.
This returns an `SAMPLING_SPEED` enum with value `HIGH_SPEED`.

Equivalently, `speed.setValue("high")` takes the spring represtation
as an argument an sets the enum value accordingly, whereas
`speed.setEnumValue(HIGH_SPEED)` allows you to set the enum value
directly.


## Actions

As we have seen above for the help action, menu items do not only
configure parameter values. They can also trigger some action by
executing some code. The help action, for example, prints a help
message on the serial stream.

Let's now have a look at the configuration menu which contains four
actions to manage configuration files.


### Configuration menu

Enter `h` and then `3` to select the configuration menu:

```txt
Configuration:
  1) Print configuration
  2) Save configuration
  3) Load configuration
  4) Erase configuration
  Select [1]: 
```

This menu contains four actions that allow you to print, save, load and
erase the configuration.

Enter `1` to print the current configuration:

```txt
Settings:
  Path:      data/
  Recording: recording.wav
  FileTime:  300s
Analog input:
  SamplingRate:  500.0kHz
  SamplingSpeed: medium
```

`2` saves the current configuration to the SD card. The content of the
configuration file looks exactly like the text we just have
printed. This output is compatible with YAML files, if you care. But
more importantly, this output format is well readable by humans, and
can be easily edited in any text editor. If there is already a
configuration file, you get a confirmation message like this:

```txt
Configuration file "micro.cfg" already exists on SD card.
Do you want to overwrite the configuration file? [Y/n]
```

Hit `y` or `enter` (the capital `Y` indicates the default) to
overwrite the existing configuration file.

"Load configuration" reads the configuration file and overwrites the
current configuration. This is why this action also asks for
confirmation first.

"Erase configuration" erases the configuration file on SD card after
confirmation.


### Firmware menu

The firmware menu supports uploading of firmware (hex files) from SD
card. This way, firmware updates can be uploaded without the need to
compile everything with Arduino IDE.

```txt
Firmware:
  1) List available updates
  2) Update firmware
  Select [1]: 
```

The first action simply lists hex files in the root directory of the SD
card. The output looks like this:

```txt
Available firmware files on SD card:
  R4-logger.ino.hex
  writeconfig.ino.hex
```

Use the "Update firmware" action to actually, well, update the
firmware. This action first lists the hex files, from which you then
choose the one you want to use by entering the respective number:

```txt
Available firmware files on SD card:
- 1) R4-logger.ino.hex
- 2) writeconfig.ino.hex

Select a firmfile file [1]: 
```

The action confirms your selection and then asks you, whether you are
really sure to update the firmware:

```txt
Selected "R4-logger.ino.hex" for firmware update

WARNING: a firmware update could make your device unusable!
WARNING: make sure that your device stays powered during the entire firmware update!

Do you really want to update the firmware? [y/N] 
```

Only if you enter `y`, a firmware update is attempted:

```txt
Successfully opened firmware file "R4-logger.ino.hex".

Updating firmware:
- initializing flash buffer ...
- created flash buffer = 8024K FLASH (60026000 - 607FC000)
- updating frimware ...

reading hex lines...

hex file: 14283 lines 228352 bytes (60000000 - 60037C00)
new code contains correct target ID fw_teensy41
enter 14283 to flash or 0 to abort
```

Enter the displayed number to really update the firmware. Keep the
microcontroller powered on and wait until the firmware update is
completed. The uploader then reboots into the new firmware.


## Banner

You may want to call the `printMicroConfigBanner()` function right
after initializing the serial stream:

```c
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  printMicroConfigBanner();                  // print a nice banner
  ...
}
```

This prints

```txt
========================================================
 __  __ _                 ____             __ _       
|  \/  (_) ___ _ __ ___  / ___|___  _ __  / _(_) __ _ 
| |\/| | |/ __| '__/ _ \| |   / _ \| '_ \| |_| |/ _` |
| |  | | | (__| | | (_) | |__| (_) | | | |  _| | (_| |
|_|  |_|_|\___|_|  \___/ \____\___/|_| |_|_| |_|\__, |
                                                |___/ 

version 1.0.0 by Benda-Lab
--------------------------------------------------------
```

This helps a GUI listening on the serial stream to detect a new menu.
