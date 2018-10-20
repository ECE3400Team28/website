/*
 This program was adapted from the Getting Started example code.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// walls
#define bm_wall       15 << 0
#define bm_wall_east  1 << 1
#define bm_wall_north 1 << 3
#define bm_wall_west  1 << 0
#define bm_wall_south 1 << 2

// treasure
#define treasure_shift       4
#define bm_treasure          7 << 4
#define bm_treasure_none     0 << 4
#define bm_treasure_b_sq     1 << 4
#define bm_treasure_r_sq     2 << 4
#define bm_treasure_b_ci     3 << 4
#define bm_treasure_r_ci     4 << 4
#define bm_treasure_b_tr     5 << 4
#define bm_treasure_r_tr     6 << 4

// whether square explored
// allows us to say if byte > 0
#define bm_not_explored 0 << 7
#define bm_explored     1 << 7
#define explored_shift  7

// coordinates
#define xcoord_shift 12
#define ycoord_shift 8

char maze_data[9][9];

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000004ALL, 0x000000004BLL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(9600);
  printf_begin();
  //printf("\n\rRF24/examples/GettingStarted/\n\r");
  //printf("ROLE: %s\n\r",role_friendly_name[role]);
  //printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(2); // we only need 2 bytes

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  //radio.printDetails();
  Serial.println("reset");
}

void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    printf("Now sending %lu...",time);
    bool ok = radio.write( &time, sizeof(unsigned long) );

    if (ok)
      printf("ok...");
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
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }

    // Try again 1s later
    delay(1000);
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      uint16_t message;
      unsigned char coord;
      unsigned char walls;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        // spew out payloads
        done = radio.read( &message, sizeof(uint16_t) );
        //printf("Got payload %d...",coord);
        
        //done = radio.read( &walls, sizeof(unsigned char) );
        //printf("Got payload %d...",walls);

        

        int x = (message & (15 << xcoord_shift)) >> xcoord_shift;
        int y = (message & (15 << ycoord_shift)) >> ycoord_shift;
        bool north = message & bm_wall_north;
        bool south = message & bm_wall_south;
        bool east = message & bm_wall_east;
        bool west = message & bm_wall_west;
        uint8_t treasure = (message & bm_treasure);
        char buff[30];
        sprintf(buff, "%d,%d", x, y);
        Serial.print(buff);
        if (north) {
          Serial.print(",north=true");
        }
        if (south) {
          Serial.print(",south=true");
        }
        if (east) {
          Serial.print(",east=true");
        }
        if (west) {
          Serial.print(",west=true");
        }
        switch (treasure) {
          case bm_treasure_none:
            break;
          case bm_treasure_b_sq:
            Serial.print(",tshape=Square,tcolor=Blue");
            break;
          case bm_treasure_r_sq:
            Serial.print(",tshape=Square,tcolor=Red");
            break;
          case bm_treasure_b_ci:
            Serial.print(",tshape=Circle,tcolor=Blue");
            break;
          case bm_treasure_r_ci:
            Serial.print(",tshape=Circle,tcolor=Red");
            break;
          case bm_treasure_b_tr:
            Serial.print(",tshape=Triangle,tcolor=Blue");
            break;
          case bm_treasure_r_tr:
            Serial.print(",tshape=Triangle,tcolor=Red");
            break;
          default:
            break;
        }
        Serial.print("\n");

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &coord, sizeof(unsigned long) );
      //printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }

  //
  // Change roles
  //

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");

      // Become the primary receiver (pong back)
      role = role_pong_back;
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
