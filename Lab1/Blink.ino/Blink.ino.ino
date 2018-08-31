/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

#include <Servo.h>

const short LED_PIN = 11;
const short SERVO_PIN = 10;
int INPUT_PIN = A0;
Servo angrySpin;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
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
  //digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);                       // wait for a second
  //digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
  //delay(1000);                       // wait for a second
  delay(100);

  // Analog input read
  int input = analogRead(INPUT_PIN);
  Serial.println(input);

  // PWM
  analogWrite(LED_PIN, input/4);
  angrySpin.write(input/6);
}
