# User guide

The following example code is essentially the [menu](examples/menu/menu.ino)
example.

First, include the `M̀icroConfig.h` header and define the main menu
`config` with configuration file "micro.cfg" on default SD card.  Also
include `SD.h` needed for handling the configuration file on the SD
card:

```c
#include <SD.h>
#include <MicroConfig.h>

Menu config("micro.cfg", &SD);
```

Next we add a menu with name "Settings" to the main menu.  This menu
gets three parameters. One a string type defining a path on the SD
card where to store recording files. The second one is a pointer to a
string specifying a file name for the recorde data. The third one is a
floating point number defining the duration of a recording. All
parameters take as first arguments the menu where they are added to,
their name, and their value in the respective type. The string-pointer
parameter takes as a value a pointer to the character array
`filename`, that holds the actual value. Further arguments provide
additional properties of the parameters. In particular, numerical
types take minimum and maximum values, a format string, and a unit:

```c
Menu settings(config, "Settings");             // settings menu

// string parameter with max 32 characters:
StringParameter<32> path(settings,             // add it to settings menu
                         "Path",               // name
                         "recordings/");       // value
char filename[64] = "recording.wav";

// string pointer parameter:
StringPointerParameter<64> file_name(settings, "Recording",
                           	     &filename);   // value is pointer to character array

// float parameter:
NumberParameter<float> file_time(settings, "FileTime", 30.0,
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
Menu aisettings(config, "Analog input");      // analog input menu

// unit32_t parameter:
NumberParameter<uint32_t> rate(aisettings, "SamplingRate",
                               48000,         // value (in Hz)
                               1,             // minimum valid value (in Hz)
                               1000000,       // maximum valid value (in Hz)
                               "%.1f",        // format string (for kHz)
                               "Hz",          // unit of the internal value
                               "kHz");        // use this unit in user interactions
```                            

Let's also add an enum parameter to the anaog input menu.
It allows to select values from an enum type. For this, 
an array with all the enum types and an array with
corresponding strings are required:

```c
// enum parameter:
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

Finally, let's add the predefined menus for handling the configuration
file, firmware updates, and printing a help message:

```c
ConfigurationMenu configuration_menu(config, SD);  // interactively report, save, load and remove configuration file
FirmwareMenu firmware_menu(config, SD);      // menu for uploading hex files from SD card
HelpAction help_act(config, "Help");         // action showing how to use the menu
```

The main code initializes the Serial stream and the builtin SD card,
loads the configuration file from SD card (if available), and executes
the main menu. At the end of the setup() function we retrieve the
values from the configuration menu. Note the respective formatting
types.

```c
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  printMicroConfigBanner();                  // print a nice banner
  SD.begin(BUILTIN_SDCARD);                  // initialize SD card
  config.load();                             // load configuration file from SD card
  if (Serial)
    config.execute(Serial, 10000);           // execute the main menu, 10s timeout
  config.report();                           // report the parameter settings
  Serial.println();
  Serial.println("Configuration values:");
  Serial.printf("  path: %s\n", path.value());
  Serial.printf("  file name: %s\n", filename);
  Serial.printf("  file time: %g\n", file_time.value());
  Serial.printf("  sampling rate: %u\n", rate.value());
  Serial.printf("  sampling speed: %u\n", speed.value());
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

Numerical parameters support unit conversion, the user may specify a
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

`q` brings you up one level, here back to the main menu. There,
enter `2` to enter the analog input menu:

```txt
Analog input:
  1) SamplingRate: 48.0kHz
  2) SamplingSpeed: medium
  Select [1]:
```

Enter `2` again to select SamplingSpeed. You get a selection of possible values:

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

Enter `q` and then `3` to select the configuration menu:

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
  SamplingSpeed: high
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

Hit `y` or `enter` (the capital `Y` indicates the default) to
overwrite the existing configuration file.

Entering `h` brings you home to the top-level menu. Now enter `q` to
exit the configuration menu. The call to `config,report()` prints
again the current configuration. The following lines use the `value()`
function of the configuration parameters to retrieve the values in the
respective types and units:

```txt
path: recordings/
file name: recording.wav
file time: 300
sampling rate: 48000
sampling speed: 2
```

Note that for the file name we do not need to retrieve the value from
the parameter. Instead we just use the variable that has been
automagically set by the StringPointerParameter.

Nice and easy, isn't it?
