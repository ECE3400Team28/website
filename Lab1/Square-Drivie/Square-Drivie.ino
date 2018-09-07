#include <Servo.h>

Servo MotorLeft;
Servo MotorRight;

int LightData1;
int LightData2;
int LightData3;
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
     LightData1 = analogRead(LightCenter);
     LightData2 = analogRead(LightLeft);
     LightData3 = analogRead(LightRight);
     if ((LightData1 && (LightData2 || LightData3)) <= 950){
      //INSERT CODE FOR HANDLING INTERSECTIONS, will need distance or camera
     }
     else{
      return 0;
     }
}

void linefollow(){
     //Below 950 is white tape
     //Above 950 is dark
     LightData1 = analogRead(LightCenter);
     LightData2 = analogRead(LightLeft);
     LightData3 = analogRead(LightRight);
     if (LightData1 <= 950){
        return;
     }
     else if (LightData2 <=950){
           MotorRight.write(92);
           MotorLeft.write(80);
           delay(400);
           return;
     }
     else{
          MotorLeft.write(88);
          MotorRight.write(100);
          delay(400);
          return;
     }
  
}
