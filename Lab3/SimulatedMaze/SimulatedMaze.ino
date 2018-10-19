/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Protocol: 16 bits (2 bytes): (4 bits) x, (4 bits) y, [(1 bit) explored, (3 bits) treasures, (walls) 1111 NSEW] <- blocked bits are stored as a maze cell
// maze byte 1
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

// maze byte 2
// whether square explored
// allows us to say if byte > 0
#define bm_explored     0 << 7
#define bm_not_explored 1 << 7
#define explored_shift  7
// presence of other robot
#define bm_robot    0 << 1
#define bm_no_robot 1 << 1
#define robot_shift 1

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000004ALL, 0x000000004BLL };
uint8_t x = 0;
uint8_t y = 0;
uint8_t maze[9][9] = {
 {bm_not_explored | bm_treasure_none | bm_wall_north | bm_wall_south | bm_wall_west, bm_not_explored | bm_treasure_b_sq | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
 {bm_not_explored | bm_treasure_r_tr | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
 {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
 {bm_not_explored | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
 {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
 {bm_not_explored | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
 {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east},
 {bm_not_explored | bm_wall_north | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_south | bm_wall_east},
 {bm_not_explored | bm_wall_south | bm_wall_west, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_south, bm_not_explored | bm_wall_north | bm_wall_east | bm_wall_south},
};

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
role_e role = role_ping_out;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(9600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

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
  // *****REAL: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, and RF24_PA_MAX*****
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

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

  radio.printDetails();
}

void loop(void)
{
  uint8_t cell = maze[x][y];
  uint16_t coordinate = x << 4 | y;
  uint16_t message = coordinate << 8 | cell;
  Serial.println(message, BIN);
  
  //
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    printf("Now sending %lu...",message);
    bool ok = radio.write( &message, sizeof(uint16_t) );

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
      
      if (x == 8 && y == 8) {
        x = 0;
        y = 0;
      }
      else if (x%2 == 0){
        if (y == 8) x++;
        else y++;
      }
      else{
        if (y == 0) x++;
        else y--;
      }
    }

    // Try again 1s later
    delay(1000);
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
