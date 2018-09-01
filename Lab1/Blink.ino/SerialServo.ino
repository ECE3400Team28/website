#include <Servo.h>

const short SERVO_PIN = 10;
int INPUT_PIN = A0;
Servo angrySpin;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  angrySpin.attach(SERVO_PIN);
  angrySpin.write(0);
  delay(1000);
  angrySpin.write(90);
  delay(1000);
  angrySpin.write(180);
  delay(1000);
}

// the loop function runs over and over again forever
void loop() {
  delay(100);

  // Analog input read
  int input = analogRead(INPUT_PIN);
  Serial.println(input);

  // PWM
  angrySpin.write(input/6);
}
