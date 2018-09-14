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
void right(){
    MotorLeft.write(80);
    MotorRight.write(80);
    delay(720);
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(100);
    return;
}
void turnleft(){
    MotorLeft.write(100);
    MotorRight.write(100);
    delay(720);
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(100);
    return;
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
     if (LightDataC <= 950){
        if (LightDataL <= 950 || LightDataR <= 950) {
          //intersection();
          return; // do this for now
        }
        else return;
     }
     else if (LightDataL <= 950){
           // bot is veering right, so we turn it left a bit
           MotorRight.write(92);
           MotorLeft.write(80);
           delay(400);
           return;
     }
     else{
          // bot is veering left, so we turn it right a bit
          MotorRight.write(100);
          MotorLeft.write(88);
          delay(400);
          return;
     }
  
}
