#include <SD.h>
#include <MicroConfig.h>

Config config("micro.cfg", &SD);           // main menu with configuration file on SD card

Menu settings(config, "Settings");         // settings menu
StringParameter<64> Path(                     // string parameter with max 64 characters
		                     settings,            // add it to settings menu
                         "Path",              // name
                         "recordings/");      // initial value
NumberParameter<float> FileTime(              // float parameter
                                settings,     // add it to settings menu
                                "FileTime",   // name
                                30.0,         // value
                                1.0,          // minimum valid value
                                8640.0,       // maximum valid value
                                "%.0f",       // format string
                                "s");         // unit of the value

Menu aisettings(config, "Analog input");    // analog input menu
NumberParameter<uint32_t> Rate(               // unit32_t parameter
                               aisettings,    // add it to aisettings menu
			                         "SamplingRate",// name 
			                         48000,         // value (in Hz)
			                         1,             // minimum valid value (in Hz)
			                         1000000,       // maximum valid value (in Hz)
			                         "%.1f",        // format string (for kHz)
			                         "Hz",          // unit of the internal value
			                         "kHz");        // use this unit in user interactions

ConfigurationMenu configuration_menu(config, SD);  // interactively report, save, load and remove configuration file
FirmwareMenu firmware_menu(config, SD);    // menu for uploading hex files from SD card
HelpAction help_act(config, "Help");       // action showing how to use the menu


void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  printMicroConfigBanner();
  SD.begin(BUILTIN_SDCARD);
  config.load();
  if (Serial)
    config.execute(Serial, 10000);
  config.report();
  Serial.println();
  Serial.println("Configuration values:");
  Serial.printf("  path: %s\n", Path.value());
  Serial.printf("  file time: %g\n", FileTime.value());
  Serial.printf("  sampling rate: %d\n", Rate.value());
}


void loop() {
  delay(100);
}
