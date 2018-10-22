// This program should do nothing. If the servos are not properly tuned, then one or both wheels will turn.
// If this is the case, use a screwdriver to tune the servos until both are stopped.

#include <Servo.h>
Servo MotorLeft;
Servo MotorRight;

void setup() {
  // put your setup code here, to run once:
    int PWM1 = 3;
    int PWM2 = 5;
    pinMode(PWM1, OUTPUT); 
    pinMode(PWM2, OUTPUT);
    MotorLeft.attach(PWM1); 
    MotorRight.attach(PWM2);
    MotorLeft.write(90);
    MotorRight.write(90);
}

void loop() {
  // put your main code here, to run repeatedly:
}
