#include <Servo.h>

Servo Motor1;
Servo Motor2;

void setup() {
  // put your setup code here, to run once:
    int PWM1 = 3;
    int PWM2 = 5;
    pinMode(PWM1, OUTPUT);
    pinMode(PWM2, OUTPUT);
    Motor1.attach(PWM1);
    Motor2.attach(PWM2);
}

void loop() {
  // put your main code here, to run repeatedly:
    Motor1.write(180);
    Motor2.write(0);
    delay(1200);
    Motor1.write(90);
    Motor2.write(90);
    delay(600);
    turn();
}

void turn(){
    Motor1.write(180);
    Motor2.write(180);
    delay(350);
    Motor1.write(90);
    Motor2.write(90);
    delay(250);
    return;
}
