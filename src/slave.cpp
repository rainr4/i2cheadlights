#include "Wire.h"
#include <Arduino.h>
#include <stdint.h>
#include <interface.h>

#define I2C_DEV_ADDR 0x40

uint32_t i = 0;

constexpr static const int max_size = 1024;
static uint8_t command_buffer[max_size];
static int command = 0; // 0 is stop 

static void on_receive_command(int len) {
    if(len>0 && len<=max_size) {
        Wire.readBytes(command_buffer,len);
        command = *command_buffer;
    }
}
template<typename T>
static void read_command(T* out_cmd) {
    memcpy(out_cmd,command_buffer+1,sizeof(T));
}

void onReceive(int len) {
  Serial.printf("onReceive[%d]: ", len);
  while (Wire.available()) {
    Serial.write(Wire.read());
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.onReceive(onReceive);
  Wire.begin(0x40, 16, 17, 100*1000);
}

void loop() {
  
}
