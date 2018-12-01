#include <Wire.h>
//#include "help.h"

#define OV7670_I2C_ADDRESS 0x21
#define FPGA_PIN_2         5
#define FPGA_PIN_1         6
#define FPGA_PIN_0         7

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
  delay(100);
  
//  for (int i1=0; i1<15; i1++) {
//    for (int j1=0; j1<15; j1++) {
//      regs[i1][j1] = read_register_value(16*i1+j1);
//    }
//  }
  
  read_key_registers();
  
  delay(100);

//  regs[0][0] = B00000001;
//  regs[0][2] = B00000000;
  regs[0][3] |= (1 << 7) | (1 << 6);
  regs[0][7] |= (1 << 3);
  regs[0][1] = B01110000;
  regs[0][2] = B01100000;
  // com3 - bit 3 set for scaling
  //regs[0][12] |= (1 << 3);
  // com14 - bit 3 for manual scaling
  //regs[3][14] |= (1 << 3);
  
  // CLKRC - set 6 for using external clock
  regs[1][1] |= (1 << 6);
  if (wantcbar == 1) {
    // com7 : set 1 for color bar, set 2 and clear 0 for RGB. set 3 for QCIF
    regs[1][2] |= (1 << 1) | (1 << 2) | (1 << 3);
    //regs[1][2] |= (1 << 1) | (1 << 3);
    // set 3 for color bar test
    regs[4][2] |= (1 << 3);
    regs[7][0] = B10111010;
    regs[7][1] = B10110101;
  } else {
    regs[1][2] |= (1 << 2) | (1 << 3);
    regs[1][2] &= ~(1 << 1);
    //regs[4][2] &= ~(1 << 3);
    //regs[7][0] = B00111010;
    //regs[7][1] = B00110101;
  }
  regs[1][2] &= ~(1 << 0);

  // bit 1 for awb enable
  regs[1][3] |= (1 << 2);
//  regs[1][3] &= ~(1 << 1);
//  regs[1][3] |= (1 << 1) | (1 << 2) | (1 << 3);
 // regs[1][3] |= (1 << 1);
 // regs[1][3] &= ~(1 << 2);
  regs[1][4] = B00000001;
  // COM9 - clear 6:3 for 2x gain ceiling, 0 to set gain
  //regs[1][14] |= (1 << 0);
  regs[1][14] = 0;

  regs[3][15] |= (1 << 3);
  
  // 11 for 0 to ff color, bits 5:4 to 01 for RGB 565
  regs[4][0] |= (1 << 7) | (1 << 6) | (1 << 4);

  regs[4][1] |= (1 << 4) | (1 << 3);

  // RGB 444 off
//  regs[8][12] &= ~(1 << 1);
  // RGB 444 on
   regs[8][12] |= (1 << 1);

  int reg_to_write[] = {0x00, 0x01, 0x02, 0x04, 0x07, 0x0b, 0x0c, 0x10, 0x11, 0x12, 0x13, 0x14, 0x1e, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x70, 0x71, 0x8c};
//  int reg_to_write[] = {0x04, 0x0b, 0x0c, 0x10, 0x11, 0x12, 0x14, 0x1e, 0x3e, 0x40, 0x42, 0x70, 0x71, 0x8c};
   // int reg_to_write[] = {0x00, 0x03, 0x04, 0x07, 0x0c, 0x10, 0x11, 0x12, 0x13, 0x14, 0x1e, 0x40, 0x42};
//  int reg_to_write[] = {0x0c, 0x11, 0x12, 0x13, 0x14, 0x1e, 0x40, 0x42, 0x70, 0x71};
//  int reg_to_write[] = {0x02, 0x12, 0x40};
  for (int i=0; i<sizeof reg_to_write/sizeof reg_to_write[0]; i++) {
    int num = reg_to_write[i] % 16;
    int num2 = (reg_to_write[i] - num) >> 4;
    OV7670_write_register(reg_to_write[i], regs[num2][num]);
    Serial.print("Writing Reg 0x");
    Serial.print(num2, HEX);
    Serial.print(num, HEX);
    Serial.print(": ");
    Serial.println(regs[num2][num], BIN);
  }

  //OV7670_write_register(0x
  
  delay(10);
  Serial.println("Reading back registers");
  read_key_registers();

  set_color_matrix();
  
  pinMode(FPGA_PIN_2, INPUT);
  pinMode(FPGA_PIN_1, INPUT);
  pinMode(FPGA_PIN_0, INPUT);
}

void loop(){
  delay(100);
  int FPGA_read = 0;

  FPGA_read += (digitalRead(FPGA_PIN_2) << 2);
  FPGA_read += (digitalRead(FPGA_PIN_1) << 1);
  FPGA_read += (digitalRead(FPGA_PIN_0) << 0);
  Serial.print(F("Received "));
  Serial.println(FPGA_read, BIN);

  switch (FPGA_read) {
    case 0:
      break;
    case 1:
      Serial.println(F("blue square"));
      break;
    case 2:
      Serial.println(F("red square"));
      break;
    case 3:
      Serial.println(F("blue dia"));
      break;
    case 4:
      Serial.println(F("red dia"));
      break;
    case 5:
      Serial.println(F("blue tr"));
      break;
    case 6:
      Serial.println(F("red tr"));
      break;
    default:
      Serial.println(F("No treasure"));
      break;
  }
  
//  bool detectBlue = digitalRead(FPGA_PIN_B);
//  bool detectRed = digitalRead(FPGA_PIN_R);
//  if (detectBlue && detectRed) {
//    Serial.println(F("Both colors detected"));
//  } else if (detectBlue) {
//    Serial.println(F("Blue treasure detected"));
//  } else if (detectRed) {
//    Serial.println(F("Red treasure detected"));
//  } else {
//    Serial.println(F("No treasure detected"));
//  }
}

//uint16_t read_12_bits(){
//  uint16_t msg = 0;
//  for (int i = 0; i < 12; i++) {
//    msg = msg << 1;
//    msg |= digitalRead(FPGA_PIN);
//    delayMicroseconds(100);
//  }
//  return msg;
//}
//
//uint16_t read_after_pinhigh(){
//  uint16_t msg_start = read_12_bits();
//  if (__builtin_popcount(msg_start) > 10)
//    return 0;
//  //return 123;
//  //if (msg_start[11:0]
//  while(!digitalRead(FPGA_PIN))
//    delayMicroseconds(100);
//  delayMicroseconds(100);
//  return read_12_bits();
//}

///////// Function Definition //////////////
void read_key_registers(){
  int key_regs[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x07, 0x0b, 0x0c, 0x10, 0x11, 0x12, 0x13, 0x14, 0x1e, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x70, 0x71, 0x8c};
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
