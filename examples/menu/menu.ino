#include <SD.h>
#include <MicroConfig.h>

#define CFG_FILE "micro.cfg"              // name of configuration file

Configurator config;                      // main menu
Menu settings("Settings");                // settings menu
StringParameter<64> Path(settings, "Path", "recordings/");
NumberParameter<float> FileTime(settings, "FileTime", 30.0, 1.0, 8640.0, "%.0f", "s");
Menu aisettings("Analog input");          // analog input menu
NumberParameter<uint32_t> Rate(aisettings, "SamplingRate", 48000, 1, 1000000, "%.1f", "Hz", "kHz");
ConfigurationMenu configuration_menu(SD); // interactively report, save, load and remove configuration file
HelpAction help_act(config, "Help");


void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  printMicroConfigBanner();
  SD.begin(BUILTIN_SDCARD);
  config.setConfigFile(CFG_FILE);
  config.load(SD);
  if (Serial)
    config.execute(Serial, 10000);
  config.report();
}


void loop() {
  delay(100);
}

