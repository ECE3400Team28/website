#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fft

#include <FHT.h> // include the library
#include <Servo.h>
#include <SPI.h>
#include <StackArray.h>
#include "nRF24L01.h"
#include "RF24.h"
//#include "printf.h"

Servo MotorLeft;
Servo MotorRight;
int LightDataC;
int LightDataL;
int LightDataR;
int wallRight;
int wallFront;
int wallLeft;

const int mux_sel_0 = 2;
const int mux_sel_1 = 7;
const int mux_sel_2 = 17;
const int rightWallLED = 4;
const int frontWallLED = 6;
const int leftWallLED = 18;
int PWM1 = 5;
int PWM2 = 3;
#define pin_Button   8
const int FRONTTHRESHOLD = 150;
const int RIGHTTHRESHOLD = 150;
const int LEFTTHRESHOLD  = 150;
const int LIGHT_CENTER_THRESHOLD = 200;//750;//550; // noticed that left right and middle sensors have different "thresholds", and this is super buggy when slight shadows exist.
const int LIGHT_RIGHT_THRESHOLD = 200;//750;//540;
const int LIGHT_LEFT_THRESHOLD = 200;//750;//620;

// *************** RADIO & GUI STUFF *************************************************************************************** //
// Hardware configuration
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9, 10);

// Protocol: 16 bits (2 bytes): (4 bits) x, (4 bits) y, [(1 bit) explored, (3 bits) treasures, (walls) 1111 NSEW] <- blocked bits are stored as a maze cell

// walls
const uint8_t bm_wall    =   15;
const uint8_t bm_wall_east = 2  ;
const uint8_t bm_wall_north = 8;
const uint8_t bm_wall_west  = 1;
const uint8_t bm_wall_south = 4;

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
const uint8_t bm_not_explored = 0;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000004ALL, 0x000000004BLL };
uint8_t x = 0;
uint8_t y = 0;
const int rows = 9;
const int columns = 9                             ;
int explored = 0;
uint8_t maze[rows][columns] = { }; // initialized with zeros
typedef enum { N = 0, S = 2, E = 1, W = 3 } facing_direction;
facing_direction current_dir = S;

const uint8_t bm_walltypes[4] = { 8, 2, 4, 1 };

// *************** MUST HAVE BARRIERS ALL AROUND SO IT DOESN'T FALSELY THINK SOMETHING IS IN FRONT OF IT ******************* //
void setup() {
  // put your setup code here, to run once:
  pinMode(pin_Button, INPUT);
  
  pinMode(leftWallLED, OUTPUT);
  pinMode(rightWallLED, OUTPUT);
  pinMode(frontWallLED, OUTPUT);

  int setup_ctr = 0;
  
  //pinMode(A4, INPUT);           //USED for microphone input
  Serial.begin(115200); // use the serial port

  // remove wall sensors from 5v line to prevent weird interference

  // wait for either microphone 660Hz or button input
  while (!readSignal() && !digitalRead(pin_Button) == HIGH) {
    Serial.println(F("no input"));
    delay(10);
    setup_ctr ++;
    if (setup_ctr == 10) {
      digitalWrite(rightWallLED, HIGH);
      digitalWrite(frontWallLED, LOW);
    } else if (setup_ctr == 2*10) {
      digitalWrite(rightWallLED, LOW);
      digitalWrite(frontWallLED, HIGH);
      setup_ctr = 0;
    }
  }
  digitalWrite(rightWallLED, LOW);
  digitalWrite(frontWallLED, LOW);
  
  pinMode(A0, INPUT);           //ADC for other robot FFT detection
  
  pinMode(PWM1, OUTPUT); 
  pinMode(PWM2, OUTPUT); 
  pinMode(A5, INPUT);           //MUX output 
  // pinMode(detectRobotLED, OUTPUT);
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
  radio.setRetries(15, 15);
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
  radio.openReadingPipe(1, pipes[1]);

  // Start listening
  radio.startListening();

  Serial.println(F("setup done!"));

  // Dump the configuration of the rf unit for debugging
  //radio.printDetails();

  delay(100);
}

StackArray<uint8_t> greedy_path;
StackArray<uint8_t> reversed_greedy_path;
bool visited[rows][columns] = {};
bool found = false;

void dfs(uint8_t xCoor, uint8_t yCoor) {
  // check if location is valid and unexplored
  if (maze[xCoor][yCoor] == 0 && xCoor>=0 && yCoor >=0 && xCoor < rows && yCoor < columns) {
    // calculate path to location
    memset(visited, 0, sizeof(visited));
    greedy(xCoor, yCoor, x, y);
    found = false;
    while (!greedy_path.isEmpty()) {
      reversed_greedy_path.push(greedy_path.pop());
    }
    // move to location using path returned by greedy
    while (!reversed_greedy_path.isEmpty()){
      uint8_t nextX = reversed_greedy_path.pop();
      uint8_t nextY = reversed_greedy_path.pop();
//      Serial.println(nextX);
//      Serial.println(nextY);
      moveOne(nextX, nextY);
    }
    explore();
    if (explored == rows*columns){
      // we've explored the entire maze! 
      while(1){
        MotorLeft.write(90);
        MotorRight.write(90);
      }
    }
    if (current_dir == S) {
        if (maze[xCoor+1][yCoor] == 0 && xCoor+1 >= 0 && yCoor >= 0 && xCoor+1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_south)) {
          dfs(xCoor+1, yCoor);
        }
        if (maze[xCoor][yCoor-1] == 0 && xCoor >= 0 && yCoor-1 >= 0 && xCoor < rows && yCoor-1 < columns && !(maze[xCoor][yCoor] & bm_wall_west)) {
          dfs(xCoor, yCoor-1);
        }
        if (maze[xCoor][yCoor+1] == 0 && xCoor >= 0 && yCoor+1 >= 0 && xCoor < rows && yCoor+1 < columns && !(maze[xCoor][yCoor] & bm_wall_east)) {
          dfs(xCoor, yCoor+1);
        }
        if (maze[xCoor-1][yCoor] == 0 && xCoor-1 >= 0 && yCoor >= 0 && xCoor-1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_north)) {
          dfs(xCoor-1, yCoor);
        }
    } else if (current_dir == N) {
        if (maze[xCoor-1][yCoor] == 0 && xCoor-1 >= 0 && yCoor >= 0 && xCoor-1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_north)) {
          dfs(xCoor-1, yCoor);
        }
        if (maze[xCoor][yCoor+1] == 0 && xCoor >= 0 && yCoor+1 >= 0 && xCoor < rows && yCoor+1 < columns && !(maze[xCoor][yCoor] & bm_wall_east)) {
          dfs(xCoor, yCoor+1);
        }
        if (maze[xCoor][yCoor-1] == 0 && xCoor >= 0 && yCoor-1 >= 0 && xCoor < rows && yCoor-1 < columns && !(maze[xCoor][yCoor] & bm_wall_west)) {
          dfs(xCoor, yCoor-1);
        }
        if (maze[xCoor+1][yCoor] == 0 && xCoor+1 >= 0 && yCoor >= 0 && xCoor+1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_south)) {
          dfs(xCoor+1, yCoor);
        }
    } else if (current_dir == E) {
        if (maze[xCoor][yCoor+1] == 0 && xCoor >= 0 && yCoor+1 >= 0 && xCoor < rows && yCoor+1 < columns && !(maze[xCoor][yCoor] & bm_wall_east)) {
          dfs(xCoor, yCoor+1);
        }
        if (maze[xCoor+1][yCoor] == 0 && xCoor+1 >= 0 && yCoor >= 0 && xCoor+1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_south)) {
          dfs(xCoor+1, yCoor);
        }
        if (maze[xCoor-1][yCoor] == 0 && xCoor-1 >= 0 && yCoor >= 0 && xCoor-1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_north)) {
          dfs(xCoor-1, yCoor);
        }
        if (maze[xCoor][yCoor-1] == 0 && xCoor >= 0 && yCoor-1 >= 0 && xCoor < rows && yCoor-1 < columns && !(maze[xCoor][yCoor] & bm_wall_west)) {
          dfs(xCoor, yCoor-1);
        }
    } else if (current_dir == W) {
        if (maze[xCoor][yCoor-1] == 0 && xCoor >= 0 && yCoor-1 >= 0 && xCoor < rows && yCoor-1 < columns && !(maze[xCoor][yCoor] & bm_wall_west)) {
          dfs(xCoor, yCoor-1);
        }
        if (maze[xCoor-1][yCoor] == 0 && xCoor-1 >= 0 && yCoor >= 0 && xCoor-1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_north)){
          dfs(xCoor-1, yCoor);
        }
        if (maze[xCoor+1][yCoor] == 0 && xCoor+1 >= 0 && yCoor >= 0 && xCoor+1 < rows && yCoor < columns && !(maze[xCoor][yCoor] & bm_wall_south)) {
          dfs(xCoor+1, yCoor);
        }
        if (maze[xCoor][yCoor+1] == 0 && xCoor >= 0 && yCoor+1 >= 0 && xCoor < rows && yCoor+1 < columns && !(maze[xCoor][yCoor] & bm_wall_east)) {
          dfs(xCoor, yCoor+1);
        }
    }
  }
}

/***
 * Given a valid coordinate (that we are not at currently) from the DFS, calculates a greedy path to that coordinate using a Manhattan distance heuristic, saving the path in a stack (greedy_path)
 */
void greedy(uint8_t goalX, uint8_t goalY, uint8_t xCoor, uint8_t yCoor) {
  // add current node to path
  greedy_path.push(xCoor);
  greedy_path.push(yCoor);
//  Serial.println(xCoor);
//  Serial.println(yCoor);
//  Serial.println(goalX);
//  Serial.println(goalY);
  if (xCoor == goalX && yCoor == goalY) {
    found = true;
    return;
  }
  // add nodes that we've explored and no wall blocking and not visited OR are the goal, in order of least to greatest cost
  int costN = -1;
  int costS = -1;
  int costE = -1;
  int costW = -1;
  if (xCoor+1 < rows && !visited[xCoor+1][yCoor] && (maze[xCoor+1][yCoor] > 0 || (xCoor+1 == goalX && yCoor == goalY)) && !(maze[xCoor][yCoor] & bm_wall_south)) {
    costS = abs(xCoor+1-goalX) + abs(yCoor-goalY);
    //Serial.println("lol1");
    visited[xCoor+1][yCoor] = true;
  }
  if (yCoor-1 >= 0 && !visited[xCoor][yCoor-1] && (maze[xCoor][yCoor-1] > 0 || (xCoor == goalX && yCoor-1 == goalY)) && !(maze[xCoor][yCoor] & bm_wall_west)) {
    costW = abs(xCoor-goalX) + abs(yCoor-1-goalY);
    //Serial.println("lol2");
    visited[xCoor][yCoor-1] = true;
  }
  if (yCoor+1 < columns && !visited[xCoor][yCoor+1] && (maze[xCoor][yCoor+1] > 0 || (xCoor == goalX && yCoor+1 == goalY)) && !(maze[xCoor][yCoor] & bm_wall_east)) {
    costE = abs(xCoor-goalX) + abs(yCoor+1-goalY);
    //Serial.println("lol3");
    visited[xCoor][yCoor+1] = true;
  }
  if (xCoor-1 >= 0 && !visited[xCoor-1][yCoor] && (maze[xCoor-1][yCoor] > 0 || (xCoor-1 == goalX && yCoor == goalY)) && !(maze[xCoor][yCoor] & bm_wall_north)) {
    costN = abs(xCoor-1-goalX) + abs(yCoor-goalY);
    //Serial.println("lol4");
    visited[xCoor-1][yCoor] = true;
  }
  facing_direction min_dir = NULL;
  
  while (!found && (costN != -1 || costS != -1 || costE != -1 || costW != -1)) { // there is still a valid direction we haven't searched
    // find min cost
    int minimum = 1000;
    if (costN != -1 && costN < minimum) {
      minimum = costN;
      min_dir = N;
    }
    if (costS != -1 && costS < minimum) {
      minimum = costS;
      min_dir = S;
    }
    if (costE != -1 && costE < minimum) {
      minimum = costE;
      min_dir = E;
    }
    if (costW != -1 && costW < minimum) {
      minimum = costW;
      min_dir = W;
    }
    // call greedy
    //Serial.println(min_dir);
    if (min_dir == S) {
      costS = -1;
      greedy(goalX, goalY, xCoor+1, yCoor);
    }
    else if (min_dir == W) {
      costW = -1;
      greedy(goalX, goalY, xCoor, yCoor-1);
    }
    else if (min_dir == E) {
      costE = -1;
      greedy(goalX, goalY, xCoor, yCoor+1);
    }
    else if (min_dir == N) {
      costN = -1;
      greedy(goalX, goalY, xCoor-1, yCoor);
    }
  }
  if (!found) {
    // remove current loc from stack
    greedy_path.pop();
    greedy_path.pop();
  }
}

void loop() {
    //Serial.println(F("exploring"));
    explore();
    dfs(1, 0);
    dfs(0, 1);
    MotorRight.write(90);
    MotorLeft.write(90);
    while(1){}
}

void moveOne(uint8_t xCoor, uint8_t yCoor) {
  // turn to face the correct direction: the next node should be one tile away.
  if (xCoor == x && yCoor == y) {
    //Serial.println("already here");
    // we are already at the location
    MotorLeft.write(90);
    MotorRight.write(90);
    return;
  }
  int x_diff = xCoor - x;
  int y_diff = yCoor - y; 
  facing_direction dir_to_face = x_diff == 1 ? S : x_diff == -1 ? N : y_diff == 1 ? E : W;
  while (current_dir != dir_to_face) {
    if (current_dir - dir_to_face == 1 || current_dir - dir_to_face == -3) turnLeft();
    else turnRight();
  }
  // move to the next intersection
  forward();
  while (!linefollow()) {
    forward(); // keeps moving forward until reaches intersection
  }
  
  MotorLeft.write(90);
  MotorRight.write(90);
  //Serial.println(F("Arrived at destination"));
  x = xCoor;
  y = yCoor;
}

/*
    Explores the current location by detecting walls & treasures and broadcasting the information. Increments the number of explored tiles.
    Assumes the current location has not been explored yet.
*/
void explore() {
  if (maze[x][y] != 0 ) return; // we've already explored this tile
  MotorLeft.write(90);
  MotorRight.write(90);
  readMux();

  // **** Check three walls because the 4th wall is the way we came into this cell, so there's no wall anyway ****//

  // check the right wall
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

  // check the front wall
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

  // check the left wall
  if (wallLeft >= LEFTTHRESHOLD) {
    digitalWrite(leftWallLED, HIGH);
    if (current_dir == N) {
      maze[x][y] |= bm_wall_west;
    } else if (current_dir == E) {
      maze[x][y] |= bm_wall_north;
    } else if (current_dir == S) {
      maze[x][y] |= bm_wall_east;
    } else if (current_dir == W) {
      maze[x][y] |= bm_wall_south;
    }
  } else {
    digitalWrite(leftWallLED, LOW);   // turn the LED off by making the voltage LOW
  }

  maze[x][y] |= bm_explored;
  broadcast(); // keep broadcasting until successful
  explored++;
}

void forward() {
  MotorLeft.write(82);
  MotorRight.write(98);
}

// Turns robot left 90 degrees and updates the currently facing direction.
void turnLeft() {
  MotorLeft.write(80);
  MotorRight.write(80);
  delay(300); // move away from current line
  readMux();
  while (!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)) {
    // keep checking
    readMux();
  }
  MotorRight.write(88);
  MotorLeft.write(88);
  if (current_dir == 0) current_dir = (facing_direction) 3;
  else current_dir = (facing_direction) (current_dir - 1);
  return;
}

// Turns robot right 90 degrees and updates the currently facing direction.
void turnRight() {
  MotorLeft.write(100);
  MotorRight.write(100);
  delay(300); // move away from current line
  readMux();
  while (!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)) {
    // keep checking
    readMux();
  }
  MotorRight.write(92);
  MotorLeft.write(92);
  
  current_dir = (facing_direction) ((current_dir + 1) % 4);
  return;
}

/***
 * Returns true upon seeing an intersection. Else returns false, and continues to follow the line.
 */
boolean linefollow() {
  //Below LIGHTTHRESHOLD is white tape
  //Above LIGHTTHRESHOLD is dark
  readMux();
  while (detect()) {
    MotorLeft.write(90);
    MotorRight.write(90);
    delay(3000);
  }

  bool leftOnLine = LightDataL <= LIGHT_LEFT_THRESHOLD;
  bool centerOnLine = LightDataC <= LIGHT_CENTER_THRESHOLD;
  bool rightOnLine = LightDataR <= LIGHT_RIGHT_THRESHOLD;
 
  if (centerOnLine && !leftOnLine && !rightOnLine) {
    // centered
    return false;
  } else if (leftOnLine && rightOnLine) {
    forward();
    delay(250);
    return true;
  } else if (centerOnLine && leftOnLine) {
    // bot is veering right slightly, so we turn it left a bit
    MotorRight.write(95);
    MotorLeft.write(84);
    delay(30);
    return false;
  } else if (centerOnLine && rightOnLine) {
    // bot is veering left slightly, so we turn it right a bit
    MotorRight.write(96);
    MotorLeft.write(85);
    delay(30);
    return false;
  } else if (leftOnLine) {
    // bot is veering right a lot, so we turn it left more
    MotorRight.write(94); //edit
    MotorLeft.write(83);
    delay(30);
    return false;
  } else if (rightOnLine) {
    // bot is veering left a lot, so we turn it right more
    MotorRight.write(97);
    MotorLeft.write(86);
    delay(30);
    return false;
  } else {
    return false;
  }
}

// DELAYED THIS BY 2 INSTEAD OF 20 MS- SEE IF IT MAKES DIFFERENCE
void readMux() { // change this so we only read once based on the input mux select value
  // 000 Front wall
  digitalWrite(mux_sel_0, LOW);
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, LOW);
  delay(2);
  wallFront = analogRead(A5);

  // 001 Right wall
  digitalWrite(mux_sel_0, HIGH);
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, LOW);
  delay(2);
  wallRight = analogRead(A5);

  // 010 Left wall
  digitalWrite(mux_sel_0, LOW);
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, LOW);
  delay(2);
  wallLeft = analogRead(A5);

  // 011 front line
  digitalWrite(mux_sel_0, HIGH);
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, LOW);
  delay(2);
  LightDataC = analogRead(A5);

  // 100 right line
  digitalWrite(mux_sel_0, LOW);
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, HIGH);
  delay(2);
  LightDataR = analogRead(A5);

  // 101 left line
  digitalWrite(mux_sel_0, HIGH);
  digitalWrite(mux_sel_1, LOW);
  digitalWrite(mux_sel_2, HIGH);
  delay(2);
  LightDataL = analogRead(A5);

  // 110 microphon
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

  for (int i = 0 ; i < FHT_N ; i ++) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fht_input[i] = k; // put real data into even bins
    //fht_input[i+1] = 0; // set odd bins to 0 - no longer necessary with FHT
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fft
  fht_run(); // process the data in the fft
  fht_mag_log(); // take the output of the fft
  sei();

  //  Serial.println("start");
  //  for (byte i = 0 ; i < FHT_N/2 ; i++) {
  //    Serial.println(fht_log_out[i]); // send out the data
  //  }
  bool detected = false;
  for (int j = 38; j < 44; ++j) {
    if (fht_log_out[j] >= 150) {
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
  // now uses FHT library

  digitalWrite(mux_sel_0, LOW);
  digitalWrite(mux_sel_1, HIGH);
  digitalWrite(mux_sel_2, HIGH);
  delay(2);
  
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < FHT_N ; i ++) { // save 256 samples
    fht_input[i] = analogRead(A5); // put real data into even bins
    //fht_input[i+1] = 0; // set odd bins to 0 - no longer necessary for FHT
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fft
  fht_run(); // process the data in the fft
  fht_mag_log(); // take the output of the fft
  sei();
//  Serial.println(fht_log_out[19]);
  int max_other = 0;
  for (int l = 28; l < 35; ++l) {
    if (fht_log_out[l] > max_other) {
      max_other = fht_log_out[l];
    }
  }

  max_other = (max_other > 45) ? max_other : 45;
  for (int j = 18; j < 21; ++j) {
    if (fht_log_out[j] >= max_other) {
      //We have detected another robot
      // return settings to original
      return true;
    }
  }
  return false;
}

boolean broadcast() {
  uint8_t cell = maze[x][y];
  if (x == 0) {
    // robot does not see behind itself on starting square.
    cell |= bm_wall_north;
  }
  if (x == 8) {
    cell |= bm_wall_south;
  }
  if (y == 0) {
    cell |= bm_wall_west;
  }
  if (y == 8) {
    cell |= bm_wall_east;
  }
  uint16_t coordinate = x << 4 | y;
  uint16_t message = coordinate << 8 | cell;
  //Serial.println(message, BIN);

  //
  // Ping out role.  Repeatedly send the current time
  //

  // First, stop listening so we can talk.
  radio.stopListening();

  //printf("Now sending %lu...", message);
  bool ok = radio.write( &message, sizeof(uint16_t) );

//  if (ok)
//    printf("ok...\n");
//  else
//    printf("failed.\n\r");

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
    //printf("Failed, response timed out.\n\r");
    // Try again 1s later
    return false;
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    return true;
  }
}
