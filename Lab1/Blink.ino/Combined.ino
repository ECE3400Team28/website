#include <Servo.h>

const short LED_PIN = 11;
const short SERVO_PIN = 10;
int INPUT_PIN = A0;
Servo angrySpin;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_PIN as an output.
  pinMode(LED_PIN, OUTPUT);
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
  //digitalWrite(LED_PIN, HIGH);       // turn the LED on (HIGH is the voltage level)
  //delay(1000);                       // wait for a second
  //digitalWrite(LED_PIN, LOW);        // turn the LED off by making the voltage LOW
  //delay(1000);                       // wait for a second
  delay(100);

  // Analog input read
  int input = analogRead(INPUT_PIN);
  Serial.println(input);

  // PWM
  analogWrite(LED_PIN, input/4);
  angrySpin.write(input/6);
}
