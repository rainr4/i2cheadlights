#include "Wire.h"
#include <Arduino.h>
#include <stdint.h>
#include <interface.h>

#define I2C_DEV_ADDR 0x40

uint32_t i = 0;

constexpr static const int max_size = 1024;
static uint8_t command_buffer[max_size];
static int command = 0; // 0 is stop 

template<typename T>
static void read_command(T* out_cmd) {
    memcpy(out_cmd,command_buffer+1,sizeof(T));
}

void execute_fade(const cmd_fade_t* fade_cmd) {
    //need to figure out code for fade
}

static void on_receive_command(int len) {
    if(len>0 && len<=max_size) {
        Wire.readBytes(command_buffer,len);
        command = *command_buffer;

        switch (command)
        {
        case CMD_FADE:
          cmd_fade_t fade_cmd;
          read_command(&fade_cmd);
          execute_fade(&fade_cmd);
          break;
        
        default:
          Serial.println("Unknown command. ");
          break;
        }
    }
}


void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_DEV_ADDR, 22, 23, 100*1000);
  Wire.onReceive(on_receive_command);
  Serial.println("Slave rdy");
  Serial.setDebugOutput(true);
}

void loop() {
  
}