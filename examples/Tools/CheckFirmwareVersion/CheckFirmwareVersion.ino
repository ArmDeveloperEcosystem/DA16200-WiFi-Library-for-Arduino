/*
   This example checks if the firmware loaded on the DA16200 module
   is updated.

   Circuit:
   - SparkFun Qwiic WiFi Shield - DA16200 attached

   Created 17 October 2018 by Riccardo Rosario Rizzo
   Modified 8 October 2021 by Sandeep Mistry to port to DA16200

   This code is in the public domain.
*/

#include <DA16200_WiFi.h>

void setup() {
  // Initialize serial
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Print a welcome message
  Serial.println("DA16200 firmware check.");
  Serial.println();

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // Print firmware version on the module
  String fv = WiFi.firmwareVersion();
  String latestFv;
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  latestFv = WIFI_FIRMWARE_LATEST_VERSION;

  // Print required firmware version
  Serial.print("Latest firmware version available : ");
  Serial.println(latestFv);

  // Check if the latest version is installed
  Serial.println();
  if (fv >= latestFv) {
    Serial.println("Check result: PASSED");
  } else {
    Serial.println("Check result: NOT PASSED");
    Serial.println(" - The firmware version on the module does not match the");
    Serial.println("   version required by the library, you may experience");
    Serial.println("   issues or failures.");
  }
}

void loop() {
  // do nothing
}
