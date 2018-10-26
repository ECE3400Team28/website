#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x42 /*TODO: write this in hex (eg. 0xAB) */

uint8_t regs[16][16];

///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  // TODO: READ KEY REGISTERS
  read_key_registers();
  
  // useful registers:
  // 0x00                 gain
  // 0x01/0x02            blue/red gain
  // 0x07/0x10/0x04       exposure value
  // 0x11                 PLL option           0bx1xxxxx
  // 0x70/0x71            test pattern         0b10100000
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS

  regs[7][0] |= (1 << 7);
  regs[7][1] |= (1 << 7);

  OV7670_write_register(0x70, regs[7][0]);
  OV7670_write_register(0x71, regs[7][1]);
  
  Serial.println("Reading back registers");
  read_key_registers();

  set_color_matrix();
}

void loop(){
 }


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  Serial.print("Reg 0x00: ");
  Serial.println(read_register_value(0x00), BIN);
  regs[0][0] = read_register_value(0x00);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x01: ");
  regs[0][1] = read_register_value(0x01);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x02: ");
  regs[0][2] = read_register_value(0x02);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x04: ");
  regs[0][4] = read_register_value(0x04);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x07: ");
  regs[0][7] = read_register_value(0x07);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x10: ");
  regs[1][0] = read_register_value(0x10);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x11: ");
  regs[1][1] = read_register_value(0x11);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x70: ");
  regs[7][0] = read_register_value(0x70);
  Serial.println(regs[0][0], BIN);
  Serial.print("Reg 0x71: ");
  regs[7][1] = read_register_value(0x71);
  Serial.println(regs[0][0], BIN);
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
