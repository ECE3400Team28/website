#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>

Servo MotorLeft;
Servo MotorRight;
int LightDataC;
int LightDataL;
int LightDataR;
int LightCenter = A2;
int LightRight = A3;
int LightLeft = A1;

int wallRight;
int wallFront;

int rightWallLED = 13;
int frontWallLED = 12;
int SOMETHRESHOLD = 1;

int i = 0;

void setup() {
  // put your setup code here, to run once:
    pinMode(A4, INPUT);           //second wall sensor
    pinMode(8, INPUT);
    while(digitalRead(8) !=  HIGH);
    Serial.begin(115200); // use the serial port
    pinMode(A0, INPUT);           //ADC for other robot FFT detection
    int PWM1 = 3;
    int PWM2 = 5;
    pinMode(PWM1, OUTPUT); 
    pinMode(PWM2, OUTPUT); 
    pinMode(LightCenter, INPUT);  //A2
    pinMode(LightRight, INPUT);   //A3, ******WAS A0, need to change wiring 
    pinMode(LightLeft, INPUT);    //A1
    pinMode(A5, INPUT);           //USED for IR distance sensor for walls 
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rightWallLED, OUTPUT);
    pinMode(frontWallLED, OUTPUT);
    MotorLeft.attach(PWM1); 
    MotorRight.attach(PWM2);
    MotorLeft.write(90);
    MotorRight.write(90);
    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0
    DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
    // put your main code here, to run repeatedly:
    if (!detect()){
        forward();
        linefollow();
    }
    delay(20);
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
     } else if (LightDataL <= 775 && LightDataR <= 775) { //intersection code
           wallfollow();
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

void wallfollow(){
  wallRight = analogRead(A5);
  wallFront = analogRead(A4);
  if (wallRight >= SOMETHRESHOLD) digitalWrite(rightWallLED, HIGH); else digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)
  if (wallFront >= SOMETHRESHOLD) digitalWrite(frontWallLED, HIGH); else digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
  if (wallFront <= SOMETHRESHOLD && wallRight >= SOMETHRESHOLD){ //if greater than threshold there is a wall 
      //we can go straight
      return;
  }
  if (wallRight < SOMETHRESHOLD){  //nothing on the right, so we can turn right 
      turnRight();
      return;
  }
  while (wallFront >= SOMETHRESHOLD && wallRight >= SOMETHRESHOLD){
      turnLeft();
      wallRight = analogRead(A5);
      wallFront = analogRead(A4);
  }
  return;
}

boolean detect() {
    cli();  // UDRE interrupt slows this way down on arduino1.0
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
  /*
    Serial.println("start");
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      Serial.println(fft_log_out[i]); // send out the data
    }
  */
   //delay(1000);  ***** do we need this 
   for (int j = 38; j < 44; ++j) {
      if (fft_log_out[j] >= 150){
          //We have detected another robot
          MotorRight.write(90);
          MotorLeft.write(90);
          digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(1000);                       // wait for a second
          digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
          delay(1000); 
          return true;
      }
   }
   return false;        //Other robots not detected 
}

boolean readSignal() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    fft_input[i] = analogRead(A4); // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  if (fft_log_out[19] >= 50){
    return true;
  }
  return false;
}
