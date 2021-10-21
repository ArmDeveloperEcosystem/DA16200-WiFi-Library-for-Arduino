void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for the serial monitor to be opened

  SERIAL_PORT_HARDWARE_OPEN.begin(115200);

  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  delay(1);
  digitalWrite(5, HIGH);
  delay(1);
  digitalWrite(5, LOW);
  delay(100);

  SERIAL_PORT_HARDWARE_OPEN.println("AT+MCUWUDONE");
  delay(100);
  SERIAL_PORT_HARDWARE_OPEN.println("AT+CLRDPMSLPEXT");
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
