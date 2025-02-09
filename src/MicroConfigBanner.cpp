#include <MicroConfigBanner.h>


void printMicroConfigBanner(const char *software, Stream &stream) {
  stream.println("\n========================================================");
  // Generated with figlet http://www.figlet.org
  stream.println(R"( __  __ _                 ____             __ _       )");
  stream.println(R"(|  \/  (_) ___ _ __ ___  / ___|___  _ __  / _(_) __ _ )");
  stream.println(R"(| |\/| | |/ __| '__/ _ \| |   / _ \| '_ \| |_| |/ _` |)");
  stream.println(R"(| |  | | | (__| | | (_) | |__| (_) | | | |  _| | (_| |)");
  stream.println(R"(|_|  |_|_|\___|_|  \___/ \____\___/|_| |_|_| |_|\__, |)");
  stream.println(R"(                                                |___/ )");
  stream.println();
  if (software == NULL)
    software = MICROCONFIG_SOFTWARE;
  if (strlen(software) > 11 && strncmp(software, "MicroConfig", 11) == 0)
    software = software + 12;
  stream.print(software);
  stream.println(" by Benda-Lab");
  stream.println("--------------------------------------------------------");
  stream.println();
}
