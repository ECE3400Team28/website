
int wallRight;
int wallFront;
int wallLeft;

int rightWallLED = 13;
int frontWallLED = 12;
int leftWallLED  = 18;
int SOMETHRESHOLD = 600;

//#define pin_PowerMux 1

int LightCenter = A2;
int LightRight = A3;
int LightLeft = A1;
const int mux_sel_0 = 2;
const int mux_sel_1 = 7;
const int mux_sel_2 = 17;

void setup() {
  // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rightWallLED, OUTPUT);
    pinMode(frontWallLED, OUTPUT);

    pinMode(LightCenter, INPUT);  //A2
    pinMode(LightRight, INPUT);   //A0
    pinMode(LightLeft, INPUT);    //A1

    pinMode(mux_sel_0, OUTPUT);
    pinMode(mux_sel_1, OUTPUT);
    pinMode(mux_sel_2, OUTPUT);
//    pinMode(pin_PowerMux, INPUT);
    
    Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  outputWall();
  //outputLine();
}

void outputLine(){
  int LightDataC = analogRead(LightCenter);
  int LightDataL = analogRead(LightLeft);
  int LightDataR = analogRead(LightRight);
  Serial.println("Center");
  Serial.println(LightDataC);
  Serial.println("Left");
  Serial.println(LightDataL);
  Serial.println("Right");
  Serial.println(LightDataR);
  delay(100);
}

void outputWall(){
//  pinMode(pin_PowerMux, OUTPUT);
//  digitalWrite(pin_PowerMux, LOW);
//  delay(100);
//  digitalWrite(pin_PowerMux, HIGH);
//  delay(100);
//  digitalWrite(pin_PowerMux, LOW);
//  delay(100);
  
  digitalWrite(mux_sel_0, LOW);  //when 00 we read from the front wall 
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, LOW);
  delay(100);
  wallFront = analogRead(A5);\
  
  digitalWrite(mux_sel_0, HIGH);  //when 01 we read from the right wall
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, LOW);
  delay(100);
  wallRight = analogRead(A5);
 
  digitalWrite(mux_sel_0, LOW);  //when 10 we read from the left wall 
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, LOW);
  delay(100);
  wallLeft = analogRead(A5);
  
//  pinMode(pin_PowerMux, INPUT);
  Serial.println("Right");
  Serial.println(wallRight);
  Serial.println("Front");
  Serial.println(wallFront);
  Serial.println("Left");
  Serial.println(wallLeft);
  delay(100);
  
//  if (wallRight >= SOMETHRESHOLD) digitalWrite(rightWallLED, HIGH); else digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)//
//  if (wallFront >= SOMETHRESHOLD) digitalWrite(frontWallLED, HIGH); else digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
//  if (wallLeft  >= SOMETHRESHOLD) digitalWrite(leftWallLED, HIGH); else digitalWrite(leftWallLED, LOW);
//  if (wallFront <= SOMETHRESHOLD && wallRight >= SOMETHRESHOLD){ //if greater than threshold there is a wall 
//      //we can go straight
//      return;
//  }
//  if (wallRight < SOMETHRESHOLD){  //nothing on the right, so we can turn right 
//      turnRight();
//      return;
//  }
//  while (wallFront >= SOMETHRESHOLD && wallRight >= SOMETHRESHOLD){
//      turnLeft();
//      wallRight = analogRead(A5);
//      wallFront = analogRead(A4);
//  }
//  return;
}
