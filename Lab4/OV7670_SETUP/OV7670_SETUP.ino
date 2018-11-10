
#include <Wire.h>
//#include "help.h"

#define OV7670_I2C_ADDRESS 0x21 /*TODO: write this in hex (eg. 0xAB) */
#define FPGA_PIN           5

int wantcbar = 0;
uint8_t regs[16][16];

///////// Main Program //////////////
void setup() {

  Wire.begin();
  Serial.begin(9600);
  
  // TODO: READ KEY REGISTERS
  Serial.println("Start");
  
  //while(Wire.available()<1);
  delay(100);

  OV7670_write_register(0x12, B10000000);

  for (int i1=0; i1<15; i1++) {
    for (int j1=0; j1<15; j1++) {
      regs[i1][j1] = read_register_value(16*i1+j1);
    }
  }
  
  read_key_registers();
  
  // useful registers:
  // 0x00                 gain
  // 0x01/0x02            blue/red gain
  // 0x07/0x10/0x04       exposure value
  // 0x11                 PLL option           0bx1xxxxx
  // 0x70/0x71            test pattern         0b10100000
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS

// values at start: 0x70 was 0xX0111010
// 0x71 was 10110101
//  regs[7][0] |= (1 << 7);
//  regs[7][0] &= ~(1 << 5);
//  regs[7][1] |= (1 << 7);
//  regs[7][1] &= ~(1 << 5);

//regs[1][1] |= (1 << 6);

regs[0][1] = B10000000;
regs[0][2] = B10000000;
regs[1][1] = B01000001;
if (wantcbar == 1) {
  regs[1][2] |= (1 << 1) | (1 << 2) | (1 << 3);
  regs[4][2] |= (1 << 3);
  regs[7][0] = B10111010;
  regs[7][1] = B10110101;
} else {
  regs[1][2] |= (1 << 2) | (1 << 3);
  regs[1][2] &= ~(1 << 1);
  regs[4][2] &= ~(1 << 3);
  regs[7][0] = B00111010;
  regs[7][1] = B00110101;
}
regs[1][2] &= ~(1 << 0);
regs[1][4] = B00000001;
regs[1][14] |= (1 << 4);
regs[4][0] |= (1 << 7) | (1 << 6) | (1 << 4);
regs[8][12] &= ~(1 << 1);
//
//  Serial.print("Writing back ");
//  Serial.print(regs[0][12], BIN);
//  Serial.println(" into register 0x11");
//  
//  Serial.print("Writing back ");
//  Serial.print(regs[1][1], BIN);
//  Serial.println(" into register 0x11");
//  
//  Serial.print("Writing back ");
//  Serial.print(regs[1][2], BIN);
//  Serial.println(" into register 0x12");
//  
//  Serial.print("Writing back ");
//  Serial.print(regs[4][0], BIN);
//  Serial.println(" into register 0x40");
//  
//  Serial.print("Writing back ");
//  Serial.print(regs[4][2], BIN);
//  Serial.println(" into register 0x42");
//  
//  Serial.print("Writing back ");
//  Serial.print(regs[7][0], BIN);
//  Serial.println(" into register 0x70");
//
//  Serial.print("Writing back ");
//  Serial.print(regs[7][1], BIN);
//  Serial.println(" into register 0x71");
//  
//  Serial.print("Writing back ");
//  Serial.print(regs[8][12], BIN);
//  Serial.println(" into register 0x8c");

//  for (int i=0; i<15; i++) {
//    for (int j=0; j<16; j++) {
//      OV7670_write_register(16*i+j, regs[i][j]);
//      Serial.print("Writing Reg 0x");
//      Serial.print(i, HEX);
//      Serial.print(j, HEX);
//      Serial.print(": ");
//      regs[i][j] = read_register_value(regs[i][j]);
//      Serial.println(regs[i][j], BIN);
//    }
//  }
  OV7670_write_register(0x01, regs[0][1]);
  OV7670_write_register(0x02, regs[0][2]);
  OV7670_write_register(0x0c, regs[0][12]);
  OV7670_write_register(0x11, regs[1][1]);
  OV7670_write_register(0x12, regs[1][2]);
  OV7670_write_register(0x14, regs[1][4]);
  OV7670_write_register(0x1e, regs[1][14]);
  OV7670_write_register(0x40, regs[4][0]);
  OV7670_write_register(0x42, regs[4][2]);
  OV7670_write_register(0x70, regs[7][0]);
  OV7670_write_register(0x71, regs[7][1]);
  OV7670_write_register(0x8c, regs[8][12]);
  
  Serial.println("Reading back registers");
  read_key_registers();

  set_color_matrix();
  
  pinMode(FPGA_PIN, INPUT);
}

void loop(){
  if (wantcbar) {
    Serial.println("writing colorbar on");
    regs[4][2] |= (1 << 3);
    OV7670_write_register(0x42, regs[4][2]);
    regs[1][2] |= (1 << 1);
    OV7670_write_register(0x12, regs[1][2]);
    regs[7][0] |= (1 << 7);
    OV7670_write_register(0x42, regs[7][0]);
    regs[7][1] |= (1 << 7);
    OV7670_write_register(0x12, regs[7][1]);
  } else {
    regs[4][2] &= ~(1 << 3);
    OV7670_write_register(0x42, regs[4][2]);
    regs[1][2] &= ~(1 << 1);
    OV7670_write_register(0x12, regs[1][2]);
    regs[7][0] &= ~(1 << 7);
    OV7670_write_register(0x70, regs[7][0]);
    regs[7][1] &= ~(1 << 7);
    OV7670_write_register(0x71, regs[7][1]);
    Serial.println("writing colorbar off");
  }
  
  read_key_registers();
  delay(5000);

//  delay(80);
//  uint16_t msg_resp=read_after_pinhigh();
//  Serial.print("Received: ");
//  Serial.println(msg_resp, BIN);
}

uint16_t read_12_bits(){
  uint16_t msg = 0;
  for (int i = 0; i < 12; i++) {
    msg = msg << 1;
    msg |= digitalRead(FPGA_PIN);
    delayMicroseconds(100);
  }
  return msg;
}

uint16_t read_after_pinhigh(){
  uint16_t msg_start = read_12_bits();
  if (__builtin_popcount(msg_start) > 10)
    return 0;
  //return 123;
  //if (msg_start[11:0]
  while(!digitalRead(FPGA_PIN))
    delayMicroseconds(100);
  delayMicroseconds(100);
  return read_12_bits();
}


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/

  int key_regs[] = {0x00, 0x01, 0x02, 0x04, 0x07, 0x0b, 0x0c, 0x10, 0x11, 0x12, 0x14, 0x1e, 0x40, 0x42, 0x70, 0x71, 0x8c};
  for (int i=0; i<sizeof key_regs/sizeof key_regs[0]; i++) {
   Serial.print("Reg 0x");
   int num = key_regs[i] % 16;
   int num2 = (key_regs[i] - num) >> 4;
   Serial.print(num2, HEX);
   Serial.print(num, HEX);
   Serial.print(": ");
   regs[num2][num] = read_register_value(key_regs[i]);
   Serial.println(regs[num2][num], BIN);
  }
  
//  Serial.print("Reg 0x00: ");
//  //Serial.println(read_register_value(0x00), BIN);
//  regs[0][0] = read_register_value(0x00);
//  Serial.println(regs[0][0], BIN);
//  Serial.print("Reg 0x01: ");
//  regs[0][1] = read_register_value(0x01);
//  Serial.println(regs[0][1], BIN);
//  Serial.print("Reg 0x02: ");
//  regs[0][2] = read_register_value(0x02);
//  Serial.println(regs[0][2], BIN);
//  Serial.print("Reg 0x04: ");
//  regs[0][4] = read_register_value(0x04);
//  Serial.println(regs[0][4], BIN);
//  
//  Serial.print("Reg 0x07: ");
//  regs[0][7] = read_register_value(0x07);
//  Serial.println(regs[0][7], BIN);
//  
//  Serial.print("Reg 0x0c: COM3 ");
//  regs[0][12] = read_register_value(0x0c);
//  Serial.println(regs[0][12], BIN);
//  
//  Serial.print("Reg 0x0d: COM4 ");
//  regs[0][13] = read_register_value(0x0d);
//  Serial.println(regs[0][13], BIN);
//  
//  Serial.print("Reg 0x10: ");
//  regs[1][0] = read_register_value(0x10);
//  Serial.println(regs[1][0], BIN);
//  Serial.print("Reg 0x11: ");
//  regs[1][1] = read_register_value(0x11);
//  Serial.println(regs[1][1], BIN);
//  Serial.print("Reg 0x12: COM7 ");
//  regs[1][2] = read_register_value(0x12);
//  Serial.println(regs[1][2], BIN);
//  Serial.print("Reg 0x1E: ");
//  regs[1][14] = read_register_value(0x1e);
//  Serial.println(regs[1][14], BIN);
//  Serial.print("Reg 0x70: ");
//  regs[7][0] = read_register_value(0x70);
//  //Serial.println(regs[7][0]);
//  Serial.println(regs[7][0], BIN);
//  Serial.print("Reg 0x40: ");
//  regs[4][0] = read_register_value(0x40);
//  //Serial.println(regs[7][0]);
//  Serial.println(regs[4][0], BIN);
//  Serial.print("Reg 0x42: ");
//  regs[4][2] = read_register_value(0x42);
//  Serial.println(regs[4][2], BIN);
//  Serial.print("Reg 0x71: ");
//  regs[7][1] = read_register_value(0x71);
//  Serial.println(regs[7][1], BIN);
//  Serial.print("Reg 0x8c: ");
//  regs[8][12] = read_register_value(0x8c);
//  Serial.println(regs[8][12], BIN);
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
