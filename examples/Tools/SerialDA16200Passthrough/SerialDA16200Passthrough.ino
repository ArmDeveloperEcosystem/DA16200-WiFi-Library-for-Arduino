int resetPin = 7;

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for the serial monitor to be opened

  SERIAL_PORT_HARDWARE_OPEN.begin(115200);

  // reset the DA16200 module
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(100);
  digitalWrite(resetPin, HIGH);
  delay(100);
}

void loop() {
  if (Serial.available()) {
    SERIAL_PORT_HARDWARE_OPEN.write(Serial.read());
  }

  if (SERIAL_PORT_HARDWARE_OPEN.available()) {
    Serial.write(SERIAL_PORT_HARDWARE_OPEN.read());
  }
}
