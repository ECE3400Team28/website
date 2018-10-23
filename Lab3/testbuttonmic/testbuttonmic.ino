#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <Servo.h>
#include <FFT.h> // include the library


void setup() {
  // put your setup code here, to run once:
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  Serial.begin(115200); // use the serial port
  while(!readSignal() && digitalRead(8) !=  HIGH) {
    delay(100);
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("p0: ");
  Serial.println(digitalRead(0));
  delay(100);
  Serial.print("p1: ");
  Serial.println(digitalRead(1));
  delay(30);
  Serial.print("p2: ");
  Serial.println(digitalRead(2));
  delay(10);
  Serial.print("p3: ");
  Serial.println(digitalRead(3));
  delay(1);
  Serial.print("p4: ");
  Serial.println(digitalRead(4));
  delay(1);
  Serial.print("p5: ");
  Serial.println(digitalRead(5));
  delay(1);
  Serial.print("p6: ");
  Serial.println(digitalRead(6));
  delay(1);
  Serial.print("p7: ");
  Serial.println(digitalRead(7));
  delay(1);
  Serial.print("p8: ");
  Serial.println(digitalRead(8));
  delay(300);
  
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
