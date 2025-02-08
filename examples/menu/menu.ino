#include <SD.h>
#include <MicroConfig.h>

#define CFG_FILE "micro.cfg"             // name of configuration file

Configurator config;                     // main menu
Configurable settings("Settings");       // settings sub menu
StringParameter<64> Path(settings, "Path", "recordings/");
NumberParameter<float> FileTime(settings, "FileTime", 30.0, 1.0, 8640.0, "%.0f", "s");
Configurable aisettings("Analog input");  // analog input sub menu
NumberParameter<uint32_t> Rate(aisettings, "SamplingRate", 48000, 1, 1000000, "%.1f", "Hz", "kHz");


void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  SD.begin(BUILTIN_SDCARD);
  config.setConfigFile(CFG_FILE);
  config.load(SD);
  if (Serial)
    config.configure(Serial, 10000);
  config.report();
}


void loop() {
  delay(100);
}

