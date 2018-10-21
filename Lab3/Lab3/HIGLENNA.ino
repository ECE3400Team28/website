#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>

Servo MotorLeft;
Servo MotorRight;
int LightDataC;
int LightDataL;
int LightDataR;
const int LightCenter = A2;
const int LightRight = A3;
const int LightLeft = A1;

int wallRight;
int wallFront;

const int mux = 7;
const int rightWallLED = 11;
const int frontWallLED = 12;
const int FRONTTHRESHOLD = 270;
const int RIGHTTHRESHOLD = 200;
int LIGHT_CENTER_THRESHOLD = 480; // noticed that left right and middle sensors have different "thresholds", and this is super buggy when slight shadows exist.
int LIGHT_RIGHT_THRESHOLD = 540;
int LIGHT_LEFT_THRESHOLD = 620;
int i = 0;

// *************** MUST HAVE BARRIERS ALL AROUND SO IT DOESN'T FALSELY THINK SOMETHING IS IN FRONT OF IT ******************* //
void setup() {
  // put your setup code here, to run once:
    pinMode(8, INPUT);
        Serial.begin(115200); // use the serial port
    while(!readSignal());
    pinMode(A0, INPUT);           //ADC for other robot FFT detection
    int PWM1 = 5;
    int PWM2 = 3;
    pinMode(PWM1, OUTPUT); 
    pinMode(PWM2, OUTPUT); 
    pinMode(LightCenter, INPUT);  //A2
    pinMode(LightRight, INPUT);   //A3
    pinMode(LightLeft, INPUT);    //A1
    pinMode(A4, INPUT);           //USED for wall distance sensor for walls 
    pinMode(A5, INPUT);           //USED for wall distance sensor for walls 
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rightWallLED, OUTPUT);
    pinMode(frontWallLED, OUTPUT);
    pinMode(mux, OUTPUT);
    MotorLeft.attach(PWM1); 
    MotorRight.attach(PWM2);
    MotorLeft.write(90);
    MotorRight.write(90);
}

void loop() {
    forward();
    linefollow();
    delay(20);
}

void forward(){
    MotorLeft.write(84);
    MotorRight.write(98);
    //Serial.println("test2");
}

void turnLeft(){
    MotorLeft.write(80);
    MotorRight.write(80);
    delay(600); // move away from current line
    // added the following check for current status:
    LightDataC = analogRead(LightCenter); 
    LightDataL = analogRead(LightLeft);
    LightDataR = analogRead(LightRight);
    // end check;
    while(!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)){
       // keep checking
       LightDataC = analogRead(LightCenter);
       LightDataL = analogRead(LightLeft);
       LightDataR = analogRead(LightRight);
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    return;
}

void turnRight(){
    MotorLeft.write(100);
    MotorRight.write(100);
    delay(600); // move away from current line
    // added the following check for current status:
    LightDataC = analogRead(LightCenter);
    LightDataL = analogRead(LightLeft);
    LightDataR = analogRead(LightRight);
    // end check
    while(!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)){
       // keep checking
       LightDataC = analogRead(LightCenter);
       LightDataL = analogRead(LightLeft);
       LightDataR = analogRead(LightRight);
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    return;
}

void linefollow(){
     //Below LIGHTTHRESHOLD is white tape
     //Above LIGHTTHRESHOLD is dark
     //Serial.println(F("test3"));
     //Serial.println(TIMSK0);
     //Serial.println(ADCSRA);
     //Serial.println(ADMUX);
     //Serial.println(DIDR0);

     LightDataC = analogRead(LightCenter);
     //Serial.println(F("test5"));
     LightDataL = analogRead(LightLeft);
     //Serial.println(F("test6"));
     LightDataR = analogRead(LightRight);
     //Serial.println(F("test7"));

     bool leftOnLine = LightDataL <= LIGHT_LEFT_THRESHOLD;
     bool centerOnLine = LightDataC <= LIGHT_CENTER_THRESHOLD;
     bool rightOnLine = LightDataR <= LIGHT_RIGHT_THRESHOLD;

     
     
     if (centerOnLine && !leftOnLine && !rightOnLine) {
           // centered
           Serial.println(F("Centered"));
           return;
     } else if (leftOnLine && rightOnLine) {
           forward();
           delay(650);
           wallfollow();
           Serial.println(F("intersection"));
           return;
     } else if (centerOnLine && leftOnLine) {
           // bot is veering right slightly, so we turn it left a bit
           MotorRight.write(92);
           MotorLeft.write(83);
           Serial.println(F("Veering slightly right"));
           delay(100);
           return;
     } else if (centerOnLine && rightOnLine) {
           // bot is veering left slightly, so we turn it right a bit
           MotorRight.write(95);
           MotorLeft.write(80);
           Serial.println(F("Veering slightly left"));
           delay(100);
           return;
     } else if (leftOnLine) {
           // bot is veering right a lot, so we turn it left more
           Serial.println(F("A lot right"));
           MotorRight.write(92);
           MotorLeft.write(80);
           delay(100);
           return;
     } else if (rightOnLine) {
           // bot is veering left a lot, so we turn it right more
           Serial.println(F("A lot left"));
           MotorLeft.write(90);
           MotorRight.write(100);
           delay(100);
           return;
     } else {
      Serial.println(F("other"));
     }
}

void wallfollow(){
  digitalWrite(mux, HIGH); //when high we read from the right wall
  wallRight = analogRead(A5);
  digitalWrite(mux, LOW); //when low we read from teh front wall 
  wallFront = analogRead(A5);
  //Serial.println(wallRight);
  //Serial.println(wallFront);
  if (wallRight >= RIGHTTHRESHOLD) digitalWrite(rightWallLED, HIGH); else digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)
  if (wallFront >= FRONTTHRESHOLD) digitalWrite(frontWallLED, HIGH); else digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
  if (wallFront <= FRONTTHRESHOLD && wallRight >= RIGHTTHRESHOLD) { //if greater than threshold there is a wall 
      // following the wall: we can go straight
      return;
  }
  if (wallRight <= RIGHTTHRESHOLD){  // nothing on the right, so we can turn right 
      turnRight();
      return;
  }
  while (wallFront >= FRONTTHRESHOLD && wallRight >= RIGHTTHRESHOLD){ // blocked on both front and right
      turnLeft();
      //delay(1000);
          digitalWrite(mux, HIGH); //when high we read from the right wall
          wallRight = analogRead(A5);
          digitalWrite(mux, LOW); //when low we read from teh front wall 
           wallFront = analogRead(A5);
      if (wallRight >= RIGHTTHRESHOLD) digitalWrite(rightWallLED, HIGH); else digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)
      if (wallFront >= FRONTTHRESHOLD) digitalWrite(frontWallLED, HIGH); else digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
  }
  return;
}

boolean detect() {
    cli();  // UDRE interrupt slows this way down on arduino1.0

    int tempTIM = TIMSK0;
    int tempSRA = ADCSRA;
    int tempMUX = ADMUX;
    int tempDID = DIDR0;
    
    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc3
    DIDR0 = 0x01; // turn off the digital input for adc3

//    int startTime=millis();
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
        while(!(ADCSRA & 0x10)); // wait for adc to be ready
        ADCSRA = 0xf5; // restart adc
        byte m = ADCL; // fetch adc data
        byte j = ADCH;
        int k = (j << 8) | m; // form into an int
        k -= 0x0200; // form into a signed int
        k <<= 6; // form into a 16b signed int
        fft_input[i] = k; // put real data into even bins
        fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
  
//    Serial.println("start");
//    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
//      Serial.println(fft_log_out[i]); // send out the data
//    }
  
   //digitalWrite(LED_BUILTIN, LOW);
//   Serial.println(millis()-startTime);
   //delay(1000);  ***** do we need this 
   for (int j = 38; j < 44; ++j) {
      if (fft_log_out[j] >= 150 && fft_log_out[j] > fft_log_out[j-10]+30 && fft_log_out[j] > fft_log_out[j+10]+30){
          TIMSK0 = tempTIM;
          ADCSRA = tempSRA;
          ADMUX = tempMUX;
          DIDR0 = tempDID;
          
          return true;
      }
   }
   //Serial.println(F("dang"));
   TIMSK0 = tempTIM;
   ADCSRA = tempSRA;
   ADMUX = tempMUX;
   DIDR0 = tempDID;
   return false;        //Other robots not detected 
}

boolean readSignal() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    fft_input[i] = analogRead(A0); // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  Serial.println(fft_log_out[19]);
  if (fft_log_out[19] >= 50){
    return true;
  }
  return false;
}
