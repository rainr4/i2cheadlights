#include "Wire.h"
#include <Arduino.h>
#include <stdint.h>
#include <interface.h>

#define I2C_RHL_ADDR 0x40
#define I2C_LHL_ADDR 0x41

uint32_t i = 0;

constexpr static const int max_size = 1024;
static uint8_t command_buffer[max_size];
static int command = 0; // 0 is stop 

// call this with your CMD_XXXXX command, and the struct you want to write
template<typename T> 
static void write_command(int cmd, const T& data) {
    static_assert(sizeof(T)<max_size,"Struct is too big");
    command = cmd;
    *command_buffer= (uint8_t)cmd;
    memcpy(command_buffer+1,&data,sizeof(data));
    Wire.write(command_buffer,sizeof(T)+1);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.begin(16, 17, 100*1000);   
}

void loop() {
  delay(5000);

  // Write message to the slave
  Wire.beginTransmission(I2C_RHL_ADDR);
  Wire.printf("Hello Right Headlight! %lu", i++);
  uint8_t error = Wire.endTransmission(true);
  Serial.printf("endTransmission: %u\n", error);

  }
