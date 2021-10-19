void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for the serial monitor to be opened

  SERIAL_PORT_HARDWARE_OPEN.begin(115200);
}

void loop() {
  if (Serial.available()) {
    SERIAL_PORT_HARDWARE_OPEN.write(Serial.read());
  }

  if (SERIAL_PORT_HARDWARE_OPEN.available()) {
    Serial.write(SERIAL_PORT_HARDWARE_OPEN.read());
  }
}
