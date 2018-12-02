/*
fft_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library


const int mux_sel_0 = 2;
const int mux_sel_1 = 7;
const int mux_sel_2 = 17;

void setup() {
  pinMode(A5, INPUT);
  Serial.begin(115200); // use the serial port
  digitalWrite(mux_sel_0, LOW);
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, HIGH);
  delay(10);
  while(!readSignal() && digitalRead(8) !=  HIGH){
    Serial.println("waiting for start");
  }
  
  
  digitalWrite(mux_sel_0, HIGH);
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  int loopctr = 0;
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    loopctr++;
    bool detected = false;
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
    Serial.print("start");
    Serial.println(loopctr);
//    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
//      Serial.println(fft_log_out[i]); // send out the data
//    }
   delay(1000);
//   Serial.println("IR start");
//   for (int j = 38; j < 44; ++j) {
    for (int j = 41; j < 47; ++j) {
    Serial.print(fft_log_out[j]);
    Serial.print(", ");
    if (fft_log_out[j] >= 150){
      Serial.println("IR detect");
      for (int k = 0; k < 128; k++) {
        Serial.println(fft_log_out[k]);
      }
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//        delay(1000);                       // wait for a second
        detected=true;
      break;
    }
   }
   if (!detected) {
    Serial.println("");
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
   }
  }
}

boolean readSignal() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    fft_input[i] = analogRead(A5); // put real data into even bins
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
