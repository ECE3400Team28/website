#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fft

#include <FHT.h> // include the library
#include <Servo.h>
#include <SPI.h>
#include <StackArray.h>
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

//const int detectRobotLED = ??;
const int mux_sel_0 = 2;
const int mux_sel_1 = 7;
const int mux_sel_2 = 17;
const int rightWallLED = 4;
const int frontWallLED = 6;
const int leftWallLED = 18;
#define pin_Button   8
const int FRONTTHRESHOLD = 150;
const int RIGHTTHRESHOLD = 150;
const int LEFTTHRESHOLD  = 150;
const int LIGHT_CENTER_THRESHOLD = 600;//550; // noticed that left right and middle sensors have different "thresholds", and this is super buggy when slight shadows exist.
const int LIGHT_RIGHT_THRESHOLD = 750;//540;
const int LIGHT_LEFT_THRESHOLD = 700;//620;

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
//#define explored_shift  7

//// presence of other robot
//#define bm_robot    1 << 1
//#define bm_no_robot 0 << 1
//#define robot_shift 1

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000004ALL, 0x000000004BLL };
uint8_t x = 0;
uint8_t y = 0;
const int rows = 9;
const int columns = 9;
int explored = 0;
uint8_t maze[rows][columns] = { }; // initialized with zeros
typedef enum { N = 0, S = 2, E = 1, W = 3 } facing_direction;
facing_direction current_dir = S;

// NODE ATTEMPT #2- GLENNA
struct Node
{
  uint8_t x;
  uint8_t y;
  uint8_t cost;
  struct Node *parent;
  struct Node *next;
  struct Node *prev;
};

// *************** MUST HAVE BARRIERS ALL AROUND SO IT DOESN'T FALSELY THINK SOMETHING IS IN FRONT OF IT ******************* //
void setup() {
  // put your setup code here, to run once:
  pinMode(pin_Button, INPUT);
  //pinMode(A4, INPUT);           //USED for microphone input
  Serial.begin(115200); // use the serial port

  // remove wall sensors from 5v line to prevent weird interference

  // wait for either microphone 660Hz or button input
  while (!readSignal() && !digitalRead(pin_Button) == HIGH) {
    Serial.println(F("no input"));
    delay(10);
  }
  pinMode(A0, INPUT);           //ADC for other robot FFT detection
  int PWM1 = 5;
  int PWM2 = 3;
  pinMode(PWM1, OUTPUT); 
  pinMode(PWM2, OUTPUT); 
  pinMode(A5, INPUT);           //MUX output 
  // pinMode(detectRobotLED, OUTPUT);
  pinMode(leftWallLED, OUTPUT);
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

  delay(2000);
}

void loop() { // try not using stackarray- use doubly linked list
//  while(1){
//    readMux();
//    Serial.println(F("center"));
//    Serial.println(LightDataC);
//    Serial.println(F("right"));
//    Serial.println(LightDataR);
//    Serial.println(F("left"));
//    Serial.println(LightDataL);
//  }
    StackArray<Node> frontier;
    Serial.println(F("start DFS"));
    struct Node n;
    n.x = x;
    n.y = y;
    n.parent = NULL;
    //Node n(x, y, 0, NULL, NULL, NULL);
    frontier.push(n);
    while(!frontier.isEmpty()) {
      struct Node loc = frontier.pop();
      Serial.print(F("next loc: "));
      Serial.print(loc.x);
      Serial.println(loc.y);
      if (abs(loc.x-x) <= 1 && abs(loc.y-y) == 0 || abs(loc.x-x) == 0 && abs(loc.y-y) <= 1) {
        // we are one step away from the target location, don't need to greedy search
        Serial.println(F("one step away!"));
        if (loc.x == x && loc.y == y) {
          //we are here
          Serial.println(F("already here"));
          moveTo(&loc); // doesn't do anything but added it for consistency
        } else if (loc.x < x && !(maze[x][y] & bm_wall_north)) {
          // the location is north, and there's no wall, so I can move
          Serial.println(F("moving north"));
          struct Node next;
          next.x = x-1;
          next.y = y;
          next.parent = &loc;
          moveTo(&next);
        } else if (loc.x > x && !(maze[x][y] & bm_wall_south)) {
          // the location is south, and there's no wall, so I can move
          Serial.println(F("moving south"));
          struct Node next;
          next.x = x+1;
          next.y = y;
          next.parent = &loc;
          moveTo(&next);
        } else if (loc.y < y && !(maze[x][y] & bm_wall_west)) {
          // the location is west, and there's no wall, so I can move
          Serial.println(F("moving west"));
          struct Node next;
          next.x = x;
          next.y = y-1;
          next.parent = &loc;
          moveTo(&next);
        } else if (loc.y > y && !(maze[x][y] & bm_wall_east)) {
          // the location is east, and there's no wall, so I can move
          Serial.println(F("moving east"));
          struct Node next;
          next.x = x;
          next.y = y+1;
          next.parent = &loc;
          moveTo(&next);
        } else {
          // the location is one away, but there is a wall so I have to run algorithm to find best path
          Serial.println(F("need more than one step to reach it!"));
          moveTo(greedy(loc.x, loc.y));
        }
      } else {
        // location is more than one away, need to run algorithm
        Serial.println(F("need more than one step to reach it!"));
        moveTo(greedy(loc.x, loc.y));
      }
      // now we are at the new location, so explore it
      Serial.println(F("exploring"));
      explore();
      if (explored == rows*columns){
        // we've explored the entire maze!
        Serial.println(F("done"));
        while(1){
          MotorLeft.write(90);
          MotorRight.write(90);
        }
      }
      
      // for each action we can take (move N/S/E/W), add the nodes to the frontier. **CHANGE THIS TO ADD NODES IN ORDER DEPENDING ON CURRENT DIR**
      if (current_dir == S) {
        if (!(maze[x][y] & bm_wall_north)) {
          // there's no wall to the north
          if (x-1 >= 0 && !(maze[x-1][y])) {
            Serial.println(F("nodeN"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x-1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_east)) {
          // there's no wall to the east
          if (y+1 < columns && !(maze[x][y+1])) {
            Serial.println(F("nodeE"));
            // the new location is valid and it has not been explored 
            struct Node n_new;
            n_new.x = x;
            n_new.y = y+1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_west)) {
          // there's no wall to the west
          if (y-1 >= 0 && !(maze[x][y-1])) {
            Serial.println(F("nodeW"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x;
            n_new.y = y-1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_south)) {
          // there's no wall to the south
          if (x+1 < rows && !(maze[x+1][y])) {
            Serial.println(F("nodeS"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x+1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
      } else if (current_dir == N) {
        if (!(maze[x][y] & bm_wall_south)) {
          // there's no wall to the south
          if (x+1 < rows && !(maze[x+1][y])) {
            Serial.println(F("nodeS"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x+1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_west)) {
          // there's no wall to the west
          if (y-1 >= 0 && !(maze[x][y-1])) {
            Serial.println(F("nodeW"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x;
            n_new.y = y-1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_east)) {
          // there's no wall to the east
          if (y+1 < columns && !(maze[x][y+1])) {
            Serial.println(F("nodeE"));
            // the new location is valid and it has not been explored 
            struct Node n_new;
            n_new.x = x;
            n_new.y = y+1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_north)) {
          // there's no wall to the north
          if (x-1 >= 0 && !(maze[x-1][y])) {
            Serial.println(F("nodeN"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x-1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
      } else if (current_dir == E) {
        if (!(maze[x][y] & bm_wall_west)) {
          // there's no wall to the west
          if (y-1 >= 0 && !(maze[x][y-1])) {
            Serial.println(F("nodeW"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x;
            n_new.y = y-1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_north)) {
          // there's no wall to the north
          if (x-1 >= 0 && !(maze[x-1][y])) {
            Serial.println(F("nodeN"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x-1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_south)) {
          // there's no wall to the south
          if (x+1 < rows && !(maze[x+1][y])) {
            Serial.println(F("nodeS"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x+1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_east)) {
          // there's no wall to the east
          if (y+1 < columns && !(maze[x][y+1])) {
            Serial.println(F("nodeE"));
            // the new location is valid and it has not been explored 
            struct Node n_new;
            n_new.x = x;
            n_new.y = y+1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        } 
      } else if (current_dir == W) {
        if (!(maze[x][y] & bm_wall_east)) {
          // there's no wall to the east
          if (y+1 < columns && !(maze[x][y+1])) {
            Serial.println(F("nodeE"));
            // the new location is valid and it has not been explored 
            struct Node n_new;
            n_new.x = x;
            n_new.y = y+1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_south)) {
          // there's no wall to the south
          if (x+1 < rows && !(maze[x+1][y])) {
            Serial.println(F("nodeS"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x+1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_north)) {
          // there's no wall to the north
          if (x-1 >= 0 && !(maze[x-1][y])) {
            Serial.println(F("nodeN"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x-1;
            n_new.y = y;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
        if (!(maze[x][y] & bm_wall_west)) {
          // there's no wall to the west
          if (y-1 >= 0 && !(maze[x][y-1])) {
            Serial.println(F("nodeW"));
            // the new location is valid and it has not been explored
            struct Node n_new;
            n_new.x = x;
            n_new.y = y-1;
            n_new.parent = NULL;
            frontier.push(n_new);
          }
        }
      }
    }
    Serial.println(F("frontier is empty: should not reach this"));
    while (1) {}
    // IR
    
//    if (detect()){
//      turnLeft();
//      digitalWrite(detectRobotLED, HIGH);
//    } else {
//      digitalWrite(detectRobotLED, LOW);
//    }
}

/*
    Uses greedy search to find the shortest path of explored tiles if stuck.
*/
struct Node* greedy(uint8_t loc_x, uint8_t loc_y) {
  //StackArray<Node> frontier_g;
  Serial.println(F("calculating path"));
  bool visited[rows][columns] = { }; // initialized with zeros
  uint8_t heuristic = abs(x - loc_x) + abs(y - loc_y);
  struct Node *rootNode = NULL;
  struct Node *lastNode = NULL;
  struct Node first_node;
  addNode(&first_node, NULL, x, y, heuristic, &rootNode, &lastNode);
  while (rootNode) {
    struct Node *loc = findAndReturnMin(&rootNode);
    Serial.println(loc->x);
    Serial.println(loc->y);
    Serial.println(loc_x);
    Serial.println(loc_y);
    // remove this location from the search list
    if (loc == rootNode && loc == lastNode) {
      rootNode = NULL;
      lastNode = NULL;
    } else if (loc == rootNode) {
      rootNode = loc->next;
      rootNode->prev = NULL;
    } else if  (loc == lastNode) {
      lastNode = loc->prev;
      lastNode->next = NULL;
    } else {
      (loc->prev)->next = loc->next;
    }
    if (!rootNode && !lastNode) {
      Serial.println(F("greedy search is null"));
    }
    visited[loc->x][loc->y] = true;
    
    if (loc->x == loc_x && loc->y == loc_y) {
      Serial.println(F("found loc"));
      return &(*loc);
    }

    // for each action we can take, add the nodes to the frontier.
    if (!(maze[loc->x][loc->y] & bm_wall_north)) {
      // there's no wall to the north
      if (loc->x - 1 >= 0 && !visited[loc->x - 1][loc->y]) {
        // the new location is valid and we have not visited it
        if (maze[loc->x - 1][loc->y] > 0 || (loc->x - 1 == loc_x && loc->y == loc_y)) {
          // we have explored this location before OR this location is the goal state
          Serial.println(F("N"));
          uint8_t heuristic = abs(loc->x-1-loc_x) + abs(loc->y-loc_y);
          struct Node n_new;
          addNode(&n_new, &(*loc), loc->x-1, loc->y, heuristic, &rootNode, &lastNode);
          Serial.print(n_new.x);
          Serial.println(n_new.y);
          if (rootNode == NULL) {
            Serial.println("yikes");
            while(1){}
          }
        }
      }
    }
    if (!(maze[loc->x][loc->y] & bm_wall_east)) {
      // there's no wall to the east
      if (loc->y + 1 < columns && !visited[loc->x][loc->y + 1] ) {
        // the new location is valid and we have not visited it
        if (maze[loc->x][loc->y + 1] > 0 || (loc->x == loc_x && loc->y + 1 == loc_y)) {
          // we have explored this location before OR this location is the goal state
          Serial.println(F("E"));
          uint8_t heuristic = abs(loc->x-loc_x) + abs(loc->y+1-loc_y);
          struct Node n_new;
          addNode(&n_new, &(*loc), loc->x, loc->y+1, heuristic, &rootNode, &lastNode);
          Serial.print(n_new.x);
          Serial.println(n_new.y);
          if (rootNode == NULL) {
            Serial.println("yikes");
            while(1){}
          }
        }
      }
    }
    if (!(maze[loc->x][loc->y] & bm_wall_south)) {
      // there's no wall to the south
      if (loc->x + 1 < rows && !visited[loc->x + 1][loc->y]) {
        // the new location is valid and it has not been explored
        if (maze[loc->x + 1][loc->y] > 0 || (loc->x + 1 == loc_x && loc->y == loc_y)) {
          // we have explored this location before OR this location is the goal state
          Serial.println(F("S"));
          uint8_t heuristic = abs(loc->x+1-loc_x) + abs(loc->y-loc_y);
          struct Node n_new;
          addNode(&n_new, &(*loc), loc->x+1, loc->y, heuristic, &rootNode, &lastNode);
          Serial.print(n_new.x);
          Serial.println(n_new.y);
          if (rootNode == NULL) {
            Serial.println("yikes");
            while(1){}
          }
        }
      }
    }
    if (!(maze[loc->x][loc->y] & bm_wall_west)) {
      // there's no wall to the west
      if (loc->y - 1 >= 0 && !visited[loc->x][loc->y - 1]) {
        // the new location is valid and it has not been explored
        if (maze[loc->x][loc->y - 1] > 0 || (loc->x == loc_x && loc->y - 1 == loc_y)) {
          // we have explored this location before OR this location is the goal state
          Serial.print(F("W"));
          uint8_t heuristic = abs(loc->x-loc_x) + abs(loc->y-1-loc_y);
          struct Node n_new;
          Serial.print(loc->x);
          Serial.println(loc->y);
          uint8_t newX = loc->x;
          uint8_t newY = loc->y;
          addNode(&n_new, &(*loc), newX, newY-1, heuristic, &rootNode, &lastNode);
          Serial.print(n_new.x);
          Serial.println(n_new.y);
          if (rootNode == NULL) {
            Serial.println("yikes");
            while(1){}
          }
        }
      }
    }
  }
}

/***
 * Creates a new linked list node and adds it to the end of the given list (need a root node and a last node)
 */
void addNode(struct Node *i, struct Node *ParentNode, uint8_t xCoor, uint8_t yCoor, uint8_t heuristic, struct Node **RootNode, struct Node **LastNode) {
  struct Node *old, *p;
  //If there is an input error, exit function
  if(!RootNode || !LastNode) {
    Serial.println(F("BADDD"));
    return;
  }
  i->x = xCoor;  //Store data and name locations
  i->y = yCoor;
  Serial.println(i->x);
  Serial.println(i->y);
  Serial.print(F("Cost: "));
  Serial.println(heuristic);
  i->cost = heuristic;
  
  if(!*LastNode) { //if empty list, this is the root element 
     i->next = NULL;        //Setup information for element
     i->prev = NULL;
     i->parent = NULL;
     *LastNode = i;        //Setup information for list
     *RootNode = i;
     Serial.println(F("the list was empty"));
     return;
  }

  //At this point, if there is no Parent, do not make node- exit function // SUUUUUUSSSSS
  if(!ParentNode) {
    Serial.println(F("there is no parent???"));
    return;
  }

   // I can prob just use lastNode to find last element?????? this is safe but slow
   //Start search at top of list
   p = *RootNode;
   old = NULL;

   while(p){ //find last element
     old = p;
     p = p->next; 
   }

   old->next = i;         //put on end
   i->next = NULL;        //Setup information for element
   i->prev = old;
   i->parent = ParentNode;
  
   *LastNode = i;         //Setup information for list
}

/***
 * Finds and returns a pointer to the node with the lowest cost. Assumes that the node passed in is not NULL.
 */
struct Node* findAndReturnMin(struct Node **RootNode) {
  struct Node *n = *RootNode;
  uint8_t min_cost = n->cost;
  struct Node *min_node = *RootNode;
  while (n) {
    if (n->cost < min_cost) {
      min_cost = n->cost;
      min_node = n;
    }
    Serial.println(F("returnmin"));
    Serial.println(n->x);
    Serial.println(n->y);
    n = n->next;
  }
  Serial.println(F("found min node"));
  return min_node;
}


/*
   Moves robot to the location described by the node. Obtains the path through the node's parent.
   Assumptions: There is an open path to the node, but it might encounter enemy robots.
*/
void moveTo(struct Node *node) {
  if (x == node->x && y == node->y) {
    Serial.println(F("made it to the location that the greedy search chose"));
    // we are already at the location
    MotorLeft.write(90);
    MotorRight.write(90);
    return;
  }
  Serial.println(F("moving"));
  StackArray<struct Node> path;
  struct Node *p = node->parent;
  if (p) {
    Serial.println(F("path exists"));
  }
  while (p) { // the path does not contain the starting (current) location.
    Serial.println(node->x);
    Serial.println(node->y);
    path.push(*node);
    node = p;
    p = node->parent;
  }
  while (!path.isEmpty()){
    struct Node next = path.pop();
    Serial.print(next.x);
    Serial.println(next.y);
    // turn to face the correct direction: the next node should be one tile away.
    int x_diff = next.x - x;
    int y_diff = next.y - y;
    facing_direction dir_to_face = x_diff == 1 ? S : x_diff == -1 ? N : y_diff == 1 ? E : W;
    while (current_dir != dir_to_face) {
      Serial.println(F("turning"));
      if (current_dir - dir_to_face == 1 || current_dir - dir_to_face == -3) turnLeft();
      else turnRight();
    }
    Serial.println(F("moving forward"));
    // move to the next intersection
    forward();
    while (!linefollow()) {
      forward(); // keeps moving forward until reaches intersection
    }
    
    MotorLeft.write(90);
    MotorRight.write(90);
    Serial.println(F("Arrived at destination"));
    x = next.x;
    y = next.y;
  }
  Serial.println(F("done"));
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
  MotorLeft.write(84);
  MotorRight.write(99);
}

// Turns robot left 90 degrees and updates the currently facing direction.
void turnLeft() {
  MotorLeft.write(80);
  MotorRight.write(80);
  delay(600); // move away from current line
  readMux();
  while (!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)) {
    // keep checking
    readMux();
  }
  MotorLeft.write(90);
  MotorRight.write(90);
  if (current_dir == 0) current_dir = (facing_direction) 3;
  else current_dir = (facing_direction) (current_dir - 1);
  return;
}

// Turns robot right 90 degrees and updates the currently facing direction.
void turnRight() {
  MotorLeft.write(100);
  MotorRight.write(100);
  delay(600); // move away from current line
  readMux();
  while (!(LightDataC <= LIGHT_CENTER_THRESHOLD && LightDataL > LIGHT_LEFT_THRESHOLD && LightDataR > LIGHT_RIGHT_THRESHOLD)) {
    // keep checking
    readMux();
  }
  MotorLeft.write(90);
  MotorRight.write(90);
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
  
  Serial.println("new data");
  Serial.println(LightDataL);
  Serial.println(LightDataC);
  Serial.println(LightDataR);
  
  bool leftOnLine = LightDataL <= LIGHT_LEFT_THRESHOLD;
  bool centerOnLine = LightDataC <= LIGHT_CENTER_THRESHOLD;
  bool rightOnLine = LightDataR <= LIGHT_RIGHT_THRESHOLD;
 
  if (centerOnLine && !leftOnLine && !rightOnLine) {
    // centered
    Serial.println(F("Centered"));
    return false;
  } else if (leftOnLine && rightOnLine) {
    forward();
    delay(650);
    Serial.println(F("intersection"));
    return true;
  } else if (centerOnLine && leftOnLine) {
    // bot is veering right slightly, so we turn it left a bit
    MotorRight.write(93);
    MotorLeft.write(83);
    Serial.println(F("Veering slightly right"));
    delay(100);
    return false;
  } else if (centerOnLine && rightOnLine) {
    // bot is veering left slightly, so we turn it right a bit
    MotorRight.write(95);
    MotorLeft.write(80);
    Serial.println(F("Veering slightly left"));
    delay(100);
    return false;
  } else if (leftOnLine) {
    // bot is veering right a lot, so we turn it left more
    Serial.println(F("A lot right"));
    MotorRight.write(90); //edit
    MotorLeft.write(80);
    delay(100);
    return false;
  } else if (rightOnLine) {
    // bot is veering left a lot, so we turn it right more
    Serial.println(F("A lot left"));
    MotorLeft.write(90);
    MotorRight.write(100);
    delay(100);
    return false;
  } else {
    Serial.println(F("other"));
    return false;
  }
}

void readMux() { // change this so we only read once based on the input mux select value
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
  delay(10);
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
  Serial.println(fht_log_out[19]);
  for (int j = 17; j < 23; ++j) {
    if (fht_log_out[j] >= 60) {
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

  printf("Now sending %lu...", message);
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
