#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <Servo.h>
#include <FFT.h> // include the library

#define pin_PowerMux 0
const int detectRobotLED = 6;
const int rightWallLED = 4;
const int frontWallLED = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  //pinMode(2, INPUT);
  pinMode(3, INPUT);
  //pinMode(4, INPUT);
  pinMode(5, INPUT);
  //pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(detectRobotLED, OUTPUT);
  pinMode(rightWallLED, OUTPUT);
  pinMode(frontWallLED, OUTPUT);
  Serial.begin(115200); // use the serial port
  while(!readSignal() && digitalRead(8) !=  HIGH) {
    delay(100);
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.print("p0: ");
//  Serial.println(digitalRead(0));
//  delay(100);
//  Serial.print("p1: ");
//  Serial.println(digitalRead(1));
//  delay(30);
//  Serial.print("p2: ");
//  Serial.println(digitalRead(2));
//  delay(10);
//  Serial.print("p3: ");
//  Serial.println(digitalRead(3));
//  delay(1);
//  Serial.print("p4: ");
//  Serial.println(digitalRead(4));
//  delay(1);
//  Serial.print("p5: ");
//  Serial.println(digitalRead(5));
//  delay(1);
//  Serial.print("p6: ");
//  Serial.println(digitalRead(6));
//  delay(1);
//  Serial.print("p7: ");
//  Serial.println(digitalRead(7));
//  delay(1);
//  Serial.print("p8: ");
//  Serial.println(digitalRead(8));
//  delay(100);

  if (detect()){
    digitalWrite(detectRobotLED, HIGH);
    digitalWrite(rightWallLED, HIGH);
    digitalWrite(frontWallLED, HIGH);
    
    Serial.println("robot detect");
  } else {
    digitalWrite(detectRobotLED, LOW);
    digitalWrite(rightWallLED, LOW);
    digitalWrite(frontWallLED, LOW);
  }
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
//  String out = "";
//  for (byte i = 0 ; i < FFT_N/2 ; i++) { 
//    Serial.println(out + fft_log_out[i] + " " + i); //send out data
//  }

  if (fft_log_out[19] >= 50 && fft_log_out[19] - fft_log_out[17] >= fft_log_out[19]*2/5 && fft_log_out[19] - fft_log_out[21] >= fft_log_out[19]*2/5){
    Serial.println("peak!");
  }
  if (fft_log_out[19] >= 50){
    Serial.println("exiting now");
    //return true;
    return false;
  }
  Serial.println("not exiting now");
  return false;
}

boolean detect() {
  pinMode(pin_PowerMux, OUTPUT);
  digitalWrite(pin_PowerMux, HIGH);
  delay(1);

  cli();  // UDRE interrupt slows this way down on arduino1.0

  int tempTIM = TIMSK0;
  int tempSRA = ADCSRA;
  int tempMUX = ADMUX;
  int tempDID = DIDR0;
  
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc3
  DIDR0 = 0x01; // turn off the digital input for adc3

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

//  Serial.println("start");
//  for (byte i = 0 ; i < FFT_N/2 ; i++) { 
//    Serial.println(fft_log_out[i]); // send out the data
//  }
  bool detected = false;
  for (int j = 38; j < 44; ++j) {
    if (fft_log_out[j] >= 150){
      //We have detected another robot
      // return settings to original
      detected = true;
      break;
    }
  }
  TIMSK0 = tempTIM;
  ADCSRA = tempSRA;
  ADMUX = tempMUX;
  DIDR0 = tempDID;
  pinMode(pin_PowerMux, INPUT);
  return detected;        //Other robots not detected 
}
