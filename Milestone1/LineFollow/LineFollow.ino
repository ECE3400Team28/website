#include <Servo.h>

Servo MotorLeft;
Servo MotorRight;

int LightDataC;
int LightDataL;
int LightDataR;
int LightCenter = A2;
int LightRight = A0;
int LightLeft = A1;
int turn[8] = {1,0,0,0,0,1,1,1};
int i = 0;

void setup() {
  // put your setup code here, to run once:
    int PWM1 = 3;
    int PWM2 = 5;
    pinMode(PWM1, OUTPUT);
    pinMode(8, INPUT);
    pinMode(PWM2, OUTPUT);
    pinMode(LightCenter, INPUT);
    pinMode(LightRight, INPUT);
    pinMode(LightLeft, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    MotorLeft.attach(PWM1); 
    MotorRight.attach(PWM2);
    MotorLeft.write(90);
    MotorRight.write(90);
    while(digitalRead(8) !=  HIGH);
    Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
    forward();
//    linefollow();
//    delay(20);
  // getValues();
}
void forward(){
    MotorLeft.write(84);
    MotorRight.write(98);
}

void turnRight(){
    MotorLeft.write(80);
    MotorRight.write(80);
    delay(600);
    while(!(LightDataC <= 775 && LightDataL > 775 && LightDataR > 775)){
     LightDataC = analogRead(LightCenter);
     LightDataL = analogRead(LightLeft);
     LightDataR = analogRead(LightRight);
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(100);
    return;
}

void turnLeft(){
    MotorLeft.write(100);
    MotorRight.write(100);
    delay(600);
     while(!(LightDataC <= 775 && LightDataL > 775 && LightDataR > 775)){
     LightDataC = analogRead(LightCenter);
     LightDataL = analogRead(LightLeft);
     LightDataR = analogRead(LightRight);
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(100);
    return;
}

void linefollow(){
     //Below 775 is white tape
     //Above 775 is dark
     LightDataC = analogRead(LightCenter);
     LightDataL = analogRead(LightLeft);
     LightDataR = analogRead(LightRight);
     Serial.println(LightDataC);
     Serial.println(LightDataL);
     Serial.println(LightDataR);
     if (LightDataC <= 775 && LightDataL > 775 && LightDataR > 775){
           // centered
           Serial.println("Centered");
           return;
     } else if (LightDataL <= 775 && LightDataR <= 775) {
            digitalWrite(LED_BUILTIN, HIGH);
           forward();
            delay(350);
            if (turn[i] == 1){
                turnRight();
            }
            else turnLeft();
            if (i ==7){
              i = 0;
            }
            else {
                 i = i + 1;
            }
           digitalWrite(LED_BUILTIN, LOW);
           return;
     } else if (LightDataC <= 775 && LightDataL <= 775){
           // bot is veering right slightly, so we turn it left a bit
           MotorRight.write(92);
           MotorLeft.write(83);
           Serial.println("Veering slightly right");
           delay(100);
           return;
     } else if (LightDataC <= 775 && LightDataR <= 775){
           // bot is veering left slightly, so we turn it right a bit
           MotorRight.write(95);
           MotorLeft.write(88);
           Serial.println("Veering slightly left");
           delay(100);
           return;
     } else if (LightDataL <= 775){
           // bot is veering right a lot, so we turn it left more
           MotorRight.write(92);
           MotorLeft.write(80);
           delay(100);
           return;
     } else if (LightDataR <= 775){
           // bot is veering left a lot, so we turn it right more
           MotorRight.write(100);
           MotorLeft.write(88);
           delay(100);
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
    MotorLeft.write(MotorSpeed);
    delay(2000);
}

void getValues(){
    LightDataC = analogRead(LightCenter);
    LightDataL = analogRead(LightLeft);
    LightDataR = analogRead(LightRight);
    Serial.println(LightDataC);
    Serial.println(LightDataL);
    Serial.println(LightDataR);
    Serial.println("DIVIDER");
}
