int INPUT_PIN = A0;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
  delay(100);

  // Analog input read
  int input = analogRead(INPUT_PIN);
  Serial.println(input);
}
