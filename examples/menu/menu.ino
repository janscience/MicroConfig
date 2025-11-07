#include <SD.h>
#include <MicroConfig.h>

Config config("micro.cfg", &SD);              // main menu with configuration file on SD card

Menu settings(config, "Settings");            // settings menu

// string parameter with max 32 characters:
StringParameter<32> path(settings,            // add it to settings menu
                         "Path",              // name
                         "recordings/");      // initial value

// string pointer parameter:
char filename[64] = "recording.wav";
StringPointerParameter<64> file_name(settings, "Recording",
                                     &filename); // value is pointer to character array

// float parameter:
NumberParameter<float> file_time(settings, "FileTime", 30.0,
                                1.0,          // minimum valid value
                                8640.0,       // maximum valid value
                                "%.0f",       // format string
                                "s");         // unit of the value

Menu aisettings(config, "Analog input");    // analog input menu

// unit32_t parameter:
NumberParameter<uint32_t> rate(aisettings, "SamplingRate",
			                         48000,         // value (in Hz)
			                         1,             // minimum valid value (in Hz)
			                         1000000,       // maximum valid value (in Hz)
			                         "%.1f",        // format string (for kHz)
			                         "Hz",          // unit of the internal value
			                         "kHz");        // use this unit in user interactions

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

ConfigurationMenu configuration_menu(config, SD);  // interactively report, save, load and remove configuration file
FirmwareMenu firmware_menu(config, SD);    // menu for uploading hex files from SD card
HelpAction help_act(config, "Help");       // action showing how to use the menu

// Message text:
MessageAction message(config, "Message", "Just a demonstration");

// Info key-value pairs:
InfoAction info(config, "Properties", "Fruit", "apple", "Color", "red", "Name", "topas");


void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  printMicroConfigBanner();
  SD.begin(BUILTIN_SDCARD);
  message.setText("Just another demonstration!");          // change the text printed out
  info.setValue("color", "green");                         // change the "color" value
  config.load();
  if (Serial)
    config.execute();
  // print reportable actions:
  Serial.println("Report:");
  config.write(Serial, Action::Report);
  Serial.println();
  // print configuration on serial:
  Serial.println("Configuration file content:");
  config.write(Serial, Action::FileOutput);
  Serial.println();
  // access configuration values:
  Serial.println("Configuration values:");
  Serial.printf("  path: %s\n", path.value());             // access configured value
  Serial.printf("  file name: %s\n", filename);            // variable has been configured
  Serial.printf("  file time: %g\n", file_time.value());
  Serial.printf("  sampling rate: %u\n", rate.value());
  Serial.printf("  sampling speed: %u\n", speed.value());
}


void loop() {
  delay(100);
}
