#include <Servo.h>

Servo MotorLeft;
Servo MotorRight;

int LightDataC;
int LightDataL;
int LightDataR;
int LightCenter = A2;
int LightRight = A0;
int LightLeft = A1;
void setup() {
  // put your setup code here, to run once:
    int PWM1 = 3;
    int PWM2 = 5;
    pinMode(PWM1, OUTPUT);
    pinMode(PWM2, OUTPUT);
    pinMode(LightCenter, INPUT);
    pinMode(LightRight, INPUT);
    pinMode(LightLeft, INPUT);
    MotorLeft.attach(PWM1); 
    MotorRight.attach(PWM2);
}

void loop() {
  // put your main code here, to run repeatedly:
     forward();
     linefollow();
}
void forward(){
    MotorLeft.write(83);
    MotorRight.write(95);
    delay(100);
}
void turnRight(){
    MotorLeft.write(80);
    MotorRight.write(80);
    delay(600);
    while (!isOnLine()) {
      delay(10);
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(100);
    return;
}
void turnleft(){
    MotorLeft.write(100);
    MotorRight.write(100);
    delay(600);
    while (!isOnLine()) {
      delay(10);
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(100);
    return;
}

bool isOnLine(){
  return LightDataC <= 950 && LightDataL > 950 && LightDataR > 950;
}

int intersection(){
    // this doens't need to check if it's in an intersection- linefollow() will do that 
    // therefore we can delete all the code below
     LightDataC = analogRead(LightCenter);
     LightDataL = analogRead(LightLeft);
     LightDataR = analogRead(LightRight);
     if ((LightDataC && (LightDataL || LightDataR)) <= 950){
      //INSERT CODE FOR HANDLING INTERSECTIONS, will need distance or camera
     }
     else{
      return 0;
     }
}

void linefollow(){
     //Below 950 is white tape
     //Above 950 is dark
     LightDataC = analogRead(LightCenter);
     LightDataL = analogRead(LightLeft);
     LightDataR = analogRead(LightRight);
     if (LightDataC <= 950 && LightDataL > 950 && LightDataR > 950){
           // centered
           return;
     } else if (LightDataC <= 950 && LightDataL <= 950 && LightDataR <= 950) {
           //intersection();
           return; // do this for now to go straight
     } else if (LightDataC <= 950 && LightDataL <= 950){
           // bot is veering right slightly, so we turn it left a bit
           MotorRight.write(92);
           MotorLeft.write(80);
           delay(400);
           return;
     } else if (LightDataC <= 950 && LightDataR <= 950){
           // bot is veering left slightly, so we turn it right a bit
           MotorRight.write(100);
           MotorLeft.write(88);
           delay(400);
           return;
     } else if (LightDataL <= 950){
           // bot is veering right a lot, so we turn it left more
           MotorRight.write(92);
           MotorLeft.write(80);
           delay(400);
           return;
     } else if (LightDataR <= 950){
           // bot is veering left a lot, so we turn it right more
           MotorRight.write(100);
           MotorLeft.write(88);
           delay(400);
           return;
     } else {
          // this is a case we did not foresee!! y i k e s
     }
}
 void motortest(){
        MotorLeft.write(90);
        MotorRight.write(90);
        Serial.println("Enter a speed");
        int MotorSpeed = Serial.parseInt();
        MotorRight.write(MotorSpeed);
        delay(2000);
 }
  void motortestu(){
        MotorLeft.write(90);
        MotorRight.write(90);
        Serial.println("Enter a speed");
        float MotorSpeed = Serial.parseFloat();
        MotorRight.writeMicroseconds(MotorSpeed); 
        delay(2000);
 }
