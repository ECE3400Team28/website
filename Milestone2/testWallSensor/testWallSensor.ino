
int wallRight;
int wallFront;

int rightWallLED = 13;
int frontWallLED = 12;
int SOMETHRESHOLD = 1;

int LightCenter = A2;
int LightRight = A3;
int LightLeft = A1;
const int mux = 7;

void setup() {
  // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rightWallLED, OUTPUT);
    pinMode(frontWallLED, OUTPUT);

    pinMode(LightCenter, INPUT);  //A2
    pinMode(LightRight, INPUT);   //A0
    pinMode(LightLeft, INPUT);    //A1

    pinMode(mux, OUTPUT);
    
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
  digitalWrite(mux, HIGH); //when high we read from the right wall
  delay(100);
  wallRight = analogRead(A5);
  digitalWrite(mux, LOW); //when low we read from teh front wall 
  delay(100);
  wallFront = analogRead(A5);
  Serial.println("Right");
  Serial.println(wallRight);
  Serial.println("Front");
  Serial.println(wallFront);
  delay(100);
  
//  if (wallRight >= SOMETHRESHOLD) digitalWrite(rightWallLED, HIGH); else digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)
//  if (wallFront >= SOMETHRESHOLD) digitalWrite(frontWallLED, HIGH); else digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
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
