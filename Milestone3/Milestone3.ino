#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

Servo MotorLeft;
Servo MotorRight;
int LightDataC;
int LightDataL;
int LightDataR;
int wallRight;
int wallFront;
int wallLeft;

const int detectRobotLED = 6;
//const int mux = 7;
const int mux_sel_0 = 2;
const int mux_sel_1 = 7;
const int mux_sel_2 = 1;
const int rightWallLED = 4;
const int frontWallLED = 6;
#define pin_Button   8
const int FRONTTHRESHOLD = 250;
const int RIGHTTHRESHOLD = 200;
const int LEFTTHRESHOLD  = 280; 
int LIGHT_CENTER_THRESHOLD = 450;//550; // noticed that left right and middle sensors have different "thresholds", and this is super buggy when slight shadows exist.
int LIGHT_RIGHT_THRESHOLD = 600;//540;
int LIGHT_LEFT_THRESHOLD = 600;//620;

// *************** RADIO & GUI STUFF *************************************************************************************** //
// Hardware configuration
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// Protocol: 16 bits (2 bytes): (4 bits) x, (4 bits) y, [(1 bit) explored, (3 bits) treasures, (walls) 1111 NSEW] <- blocked bits are stored as a maze cell

// walls 
const uint8_t bm_wall    =   15;
const uint8_t bm_wall_east = 2  ;
const uint8_t bm_wall_north=8;
const uint8_t bm_wall_west  =1;
const uint8_t bm_wall_south =4;

// treasure
#define treasure_shift 4
const uint8_t bm_treasure      = 112 ;// 7 << 4
const uint8_t bm_treasure_none = 0  ; // 0 << 4
const uint8_t bm_treasure_b_sq = 16 ; // 1 << 4
const uint8_t bm_treasure_r_sq = 32 ; // 2 << 4
const uint8_t bm_treasure_b_ci = 48 ; // 3 << 4
const uint8_t bm_treasure_r_co = 64 ; // 4 << 4
const uint8_t bm_treasure_b_tr = 80 ; // 5 << 4
const uint8_t bm_treasure_r_tr = 96 ; // 6 << 4

// whether square explored
const uint8_t bm_explored    = 128;
const uint8_t bm_not_explored= 0;
//#define explored_shift  7

//// presence of other robot
//#define bm_robot    1 << 1
//#define bm_no_robot 0 << 1
//#define robot_shift 1

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000004ALL, 0x000000004BLL };
uint8_t x = 0;
uint8_t y = 0;
//uint8_t maze[9][9] = {
// {bm_not_explored | bm_treasure_none | bm_wall_north | bm_wall_south | bm_wall_west, bm_not_explored | bm_treasure_b_sq | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
// {bm_not_explored | bm_treasure_r_tr | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
// {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
// {bm_not_explored | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
// {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
// {bm_not_explored | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
// {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
// {bm_not_explored | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
// {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east | bm_wall_south},
//};

uint8_t maze[2][3] = {
 {0, 0, 0},
 {0, 0, 0}
};

typedef enum { N = 0, S = 2, E = 1, W = 3 } facing_direction;

// The role of the current running sketch
facing_direction current_dir = S;


// *************** MUST HAVE BARRIERS ALL AROUND SO IT DOESN'T FALSELY THINK SOMETHING IS IN FRONT OF IT ******************* //
void setup() {
  // put your setup code here, to run once:
  pinMode(pin_Button, INPUT);
  //pinMode(A4, INPUT);           //USED for microphone input
  Serial.begin(115200); // use the serial port

  // remove wall sensors from 5v line to prevent weird interference

  // wait for either microphone 660Hz or button input
  while(!readSignal() && !digitalRead(pin_Button) == HIGH) {
    Serial.println(F("no input"));
    delay(10);
  }
  pinMode(A0, INPUT);           //ADC for other robot FFT detection
  int PWM1 = 5;
  int PWM2 = 3;
  pinMode(PWM1, OUTPUT); 
  pinMode(PWM2, OUTPUT); 
  pinMode(A5, INPUT);           //MUX output 
  pinMode(detectRobotLED, OUTPUT);
  pinMode(rightWallLED, OUTPUT);
  pinMode(frontWallLED, OUTPUT);
  pinMode(mux_sel_0, OUTPUT);
  pinMode(mux_sel_1, OUTPUT);
  pinMode(mux_sel_2, OUTPUT);
  MotorLeft.attach(PWM1); 
  MotorRight.attach(PWM2);
  MotorLeft.write(90);
  MotorRight.write(90);
    
  // Setup and configure rf radio
  radio.begin();
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // *****REAL: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, and RF24_PA_MAX*****
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // Set message size since we only need 2 bytes
  radio.setPayloadSize(2); 

  // Open pipes to other nodes for communication
  // Open 'our' pipe for writing, the 'other' pipe for reading
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  
  // Start listening
  radio.startListening();
  
  Serial.println(F("setup done!"));

  // Dump the configuration of the rf unit for debugging
  //radio.printDetails();
  
  delay(2000);
}

void loop() {
    Serial.println(F("start loop"));
    forward();
    linefollow();

    // IR
    
    if (detect()){
      turnLeft();
      digitalWrite(detectRobotLED, HIGH);
    } else {
      digitalWrite(detectRobotLED, LOW);
    }
}

void forward(){
    MotorLeft.write(84);
    MotorRight.write(99);
}

void turnLeft(){
    MotorLeft.write(80);
    MotorRight.write(80);
    delay(600); // move away from current line
    // added the following check for current status:
    
//    LightDataC = analogRead(LightCenter); 
//    delay(1);
//    LightDataL = analogRead(LightLeft);
//    delay(1);
//    LightDataR = analogRead(LightRight);
//    delay(1);

    readMux();
    
    // end check;
    while(!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)){
       // keep checking
//       LightDataC = analogRead(LightCenter);
//       LightDataL = analogRead(LightLeft);
//       LightDataR = analogRead(LightRight);

       readMux();
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
//    LightDataC = analogRead(LightCenter);
//    LightDataL = analogRead(LightLeft);
//    LightDataR = analogRead(LightRight);

    readMux();
    // end check
    while(!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)){
       // keep checking
//       LightDataC = analogRead(LightCenter);
//       LightDataL = analogRead(LightLeft);
//       LightDataR = analogRead(LightRight);
       readMux();
    }
    MotorLeft.write(90);
    MotorRight.write(90);
    return;
}

void linefollow(){
     //Below LIGHTTHRESHOLD is white tape
     //Above LIGHTTHRESHOLD is dark
//
//     digitalWrite(mux_sel_0, HIGH);
//     digitalWrite(mux_sel_0, HIGH);
//     digitalWrite(mux_sel_0, LOW);
//     delay(20);
//     LightDataC = analogRead(A5);
//
//     digitalWrite(mux_sel_0, HIGH);
//     digitalWrite(mux_sel_0, LOW);
//     digitalWrite(mux_sel_0, HIGH);
//     delay(20);
//     LightDataL = analogRead(A5);
//
//     digitalWrite(mux_sel_0, LOW);
//     digitalWrite(mux_sel_0, LOW);
//     digitalWrite(mux_sel_0, HIGH);
//     delay(20);
//     LightDataR = analogRead(A5);

     readMux();

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
           MotorRight.write(93);
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
  MotorLeft.write(90);
  MotorRight.write(90);

//  digitalWrite(mux_sel_0, LOW);  //when 000 we read from the front wall 
//  digitalWrite(mux_sel_1, LOW);
//  digitalWrite(mux_sel_2, LOW);
//  delay(20);
//  wallFront = analogRead(A5);
//  
//  digitalWrite(mux_sel_0, HIGH);  //when 001 we read from the right wall
//  digitalWrite(mux_sel_1, LOW);
//  digitalWrite(mux_sel_2, LOW);
//  delay(20);
//  wallRight = analogRead(A5);
// 
//  digitalWrite(mux_sel_0, LOW);  //when 010 we read from the left wall 
//  digitalWrite(mux_sel_1, HIGH);
//  digitalWrite(mux_sel_2, LOW);
//  delay(20);
//  wallLeft = analogRead(A5);

  readMux();
  
  Serial.println(wallFront);
  //Serial.println(wallRight);
  //Serial.println(wallFront);
  switch (current_dir) {
    case N:
      x--;
      break;
    case S:
      x++;
      break;
    case E:
      y++;
      break;
    case W:
      y--;
      break;
  }
 x %= 16;
 y %= 16;
  if (wallRight >= RIGHTTHRESHOLD) {
    digitalWrite(rightWallLED, HIGH);
    if (current_dir == N) {
      maze[x][y] |= bm_wall_east;
    } else if (current_dir == E) {
      maze[x][y] |= bm_wall_south;
    } else if (current_dir == S) {
      maze[x][y] |= bm_wall_west;
    } else if (current_dir == W) {
      maze[x][y] |= bm_wall_north;
    }
  } else {
    digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)
  }
  
  if (wallFront >= FRONTTHRESHOLD) {
    digitalWrite(frontWallLED, HIGH);
    if (current_dir == N) {
      maze[x][y] |= bm_wall_north;
    } else if (current_dir == E) {
      maze[x][y] |= bm_wall_east;
    } else if (current_dir == S) {
      maze[x][y] |= bm_wall_south;
    } else if (current_dir == W) {
      maze[x][y] |= bm_wall_west;
    }
  } else {
    digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
  }
  
  if (maze[x][y] >= 0) {
    // have not explored yet
    maze[x][y] |= bm_explored;
    broadcast(); // keep broadcasting until successful
  }
  
  if (wallFront <= FRONTTHRESHOLD && wallRight >= RIGHTTHRESHOLD) { //if greater than threshold there is a wall 
      // following the wall: we can go straight
      return;
  }
  if (wallRight <= RIGHTTHRESHOLD){  // nothing on the right, so we can turn right 
      turnRight();
      current_dir = (facing_direction) ((current_dir + 1) % 4);
      return;
  }
  while (wallFront >= FRONTTHRESHOLD && wallRight >= RIGHTTHRESHOLD){ // blocked on both front and right
      turnLeft();
      if (current_dir-1 < 0) current_dir = (facing_direction) 3;
      else current_dir = (facing_direction) (current_dir - 1);
      
      //delay(1000);
//
//      digitalWrite(mux, HIGH); //when high we read from the right wall
//      delay(20);
//      wallRight = analogRead(A5);
//      digitalWrite(mux, LOW);  //when low we read from the front wall 
//      delay(20);
//      wallFront = analogRead(A5);
      
      readMux();
      
      if (wallRight >= RIGHTTHRESHOLD) {
        digitalWrite(rightWallLED, HIGH);
        if (current_dir == N) {
          maze[x][y] |= bm_wall_east;
        } else if (current_dir == E) {
          maze[x][y] |= bm_wall_south;
        } else if (current_dir == S) {
          maze[x][y] |= bm_wall_west;
        } else if (current_dir == W) {
          maze[x][y] |= bm_wall_north;
        }
      } else {
        digitalWrite(rightWallLED, LOW);   // turn the LED on (HIGH is the voltage level)
      }
      
      if (wallFront >= FRONTTHRESHOLD) {
        digitalWrite(frontWallLED, HIGH);
        if (current_dir == N) {
          maze[x][y] |= bm_wall_north;
        } else if (current_dir == E) {
          maze[x][y] |= bm_wall_east;
        } else if (current_dir == S) {
          maze[x][y] |= bm_wall_south;
        } else if (current_dir == W) {
          maze[x][y] |= bm_wall_west;
        }
      } else {
        digitalWrite(frontWallLED, LOW);   // turn the LED off by making the voltage LOW
      }
    // broadcast();
  }
  return;
}

void readMux() {
  // 000 Front wall
  digitalWrite(mux_sel_0, LOW);   
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, LOW);
  delay(20);
  wallFront = analogRead(A5);
  
  // 001 Right wall
  digitalWrite(mux_sel_0, HIGH);  
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, LOW);
  delay(20);
  wallRight = analogRead(A5);
  
  // 010 Left wall
  digitalWrite(mux_sel_0, LOW);   
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, LOW);
  delay(20);
  wallLeft = analogRead(A5);
  
  // 011 front line
  digitalWrite(mux_sel_0, HIGH);  
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, LOW);
  delay(20);
  LightDataC = analogRead(A5);
  
  // 100 right line
  digitalWrite(mux_sel_0, LOW);  
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, HIGH);
  delay(20);
  LightDataR = analogRead(A5);
  
  // 101 left line
  digitalWrite(mux_sel_0, HIGH);  
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, HIGH);
  delay(20);
  LightDataL = analogRead(A5);
  
  // 110 microphone
  // 111 IR maybe?
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
  return detected;        //Other robots not detected 
}

boolean readSignal() {
  delay(10);
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    digitalWrite(mux_sel_0, LOW);   
    digitalWrite(mux_sel_1, HIGH);
    digitalWrite(mux_sel_2, HIGH);
    delay(20);
    fft_input[i] = analogRead(A5); // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  Serial.println(fft_log_out[19]);
  for (int j = 17; j < 23; ++j) {
    if (fft_log_out[j] >= 60){
      //We have detected another robot
      // return settings to original
      return true;
    }
  }
  return false;
}

boolean broadcast() {
  uint8_t cell = maze[x][y];
  uint16_t coordinate = x << 4 | y;
  uint16_t message = coordinate << 8 | cell;
  Serial.println(message, BIN);
  
  //
  // Ping out role.  Repeatedly send the current time
  //

  // First, stop listening so we can talk.
  radio.stopListening();

  printf("Now sending %lu...",message);
  bool ok = radio.write( &message, sizeof(uint16_t) );

  if (ok)
    printf("ok...\n");
  else
    printf("failed.\n\r");

  // Now, continue listening
  radio.startListening();

  // Wait here until we get a response, or timeout (250ms)
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;

  // Describe the results
  if ( timeout )
  {
    printf("Failed, response timed out.\n\r");
    // Try again 1s later
    return false;
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    return true;
  }
}
