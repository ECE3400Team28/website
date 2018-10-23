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
const int LightCenter = A2;
const int LightRight = A3;
const int LightLeft = A1;

int wallRight;
int wallFront;

const int detectRobotLED = 6;
const int mux = 7;
const int rightWallLED = 4;
const int frontWallLED = 2;
#define pin_PowerMux 0
#define pin_Button   8
const int FRONTTHRESHOLD = 200;
const int RIGHTTHRESHOLD = 140;
int LIGHT_CENTER_THRESHOLD = 800;//550; // noticed that left right and middle sensors have different "thresholds", and this is super buggy when slight shadows exist.
int LIGHT_RIGHT_THRESHOLD = 800;//540;
int LIGHT_LEFT_THRESHOLD = 800;//620;
//int i = 0;

// *************** RADIO & GUI STUFF *************************************************************************************** //
// Hardware configuration
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// Protocol: 16 bits (2 bytes): (4 bits) x, (4 bits) y, [(1 bit) explored, (3 bits) treasures, (walls) 1111 NSEW] <- blocked bits are stored as a maze cell

// walls 
#define bm_wall       15 << 0
#define bm_wall_east  1 << 1
#define bm_wall_north 1 << 3
#define bm_wall_west  1 << 0
#define bm_wall_south 1 << 2

// treasure
#define treasure_shift 4
#define bm_treasure_none     0 << 4
#define bm_treasure_b_sq     1 << 4
#define bm_treasure_r_sq     2 << 4
#define bm_treasure_b_ci     3 << 4
#define bm_treasure_r_ci     4 << 4
#define bm_treasure_b_tr     5 << 4
#define bm_treasure_r_tr     6 << 4

// whether square explored
#define bm_explored     1 << 7
#define bm_not_explored 0 << 7
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
  pinMode(A4, INPUT);           //USED for microphone input
  Serial.begin(115200); // use the serial port

  // remove wall sensors from 5v line to prevent weird interference
  pinMode(pin_PowerMux, OUTPUT);
  digitalWrite(pin_PowerMux, HIGH);

  // wait for either microphone 660Hz or button input
  while(!readSignal() && digitalRead(pin_Button) !=  HIGH) {
    Serial.println(F("no input"));
    delay(10);
  }
  pinMode(A0, INPUT);           //ADC for other robot FFT detection
  int PWM1 = 5;
  int PWM2 = 3;
  pinMode(PWM1, OUTPUT); 
  pinMode(PWM2, OUTPUT); 
  pinMode(LightCenter, INPUT);  //A2
  pinMode(LightRight, INPUT);   //A3
  pinMode(LightLeft, INPUT);    //A1
  pinMode(A5, INPUT);           //USED for wall distance sensor for walls 
  pinMode(detectRobotLED, OUTPUT);
  pinMode(rightWallLED, OUTPUT);
  pinMode(frontWallLED, OUTPUT);
  pinMode(mux, OUTPUT);
  pinMode(pin_PowerMux, INPUT);
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
    delay(20);

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
    MotorRight.write(98);
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

     LightDataC = analogRead(LightCenter);
     LightDataL = analogRead(LightLeft);
     LightDataR = analogRead(LightRight);

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
  MotorLeft.write(90);
  MotorRight.write(90);
  pinMode(pin_PowerMux, OUTPUT);
  digitalWrite(pin_PowerMux, LOW);
  digitalWrite(mux, HIGH); //when high we read from the right wall
  wallRight = analogRead(A5);
  digitalWrite(mux, LOW);  //when low we read from the front wall 
  wallFront = analogRead(A5);
  pinMode(pin_PowerMux, INPUT);
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
  
//  if (maze[x][y] & bm_not_explored > 0)
  broadcast();
//  else maze[x][y] |= bm_explored;
  
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

      pinMode(pin_PowerMux, OUTPUT);
      digitalWrite(pin_PowerMux, LOW);
      digitalWrite(mux, HIGH); //when high we read from the right wall
      wallRight = analogRead(A5);
      digitalWrite(mux, LOW);  //when low we read from the front wall 
      wallFront = analogRead(A5);
      pinMode(pin_PowerMux, INPUT);
      
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
      broadcast();
  }
  return;
}

boolean detect() {
  pinMode(pin_PowerMux, OUTPUT);
  digitalWrite(pin_PowerMux, HIGH);

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
    delay(1000);
    return false;
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    unsigned long got_time;
    radio.read( &got_time, sizeof(unsigned long) );

    // Spew it
    printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    return true;
  }
}
