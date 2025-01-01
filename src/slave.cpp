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

void fade_led(const rgbw_t& color_1, const rgbw_t& color_2, uint32_t step, uint32_t step_interval, bool ping_pong) {
    // Example implementation for fading LED
    for (uint32_t i = 0; i <= step; ++i) {
        float ratio = static_cast<float>(i) / step;
        rgbw_t current_color = {
            static_cast<uint8_t>(color_1.r + ratio * (color_2.r - color_1.r)),
            static_cast<uint8_t>(color_1.g + ratio * (color_2.g - color_1.g)),
            static_cast<uint8_t>(color_1.b + ratio * (color_2.b - color_1.b)),
            static_cast<uint8_t>(color_1.w + ratio * (color_2.w - color_1.w))
        };
        // Set LED to current_color
        // Example: setLED(current_color);
        delay(step_interval);
    }
    if (ping_pong) {
        for (uint32_t i = step; i > 0; --i) {
            float ratio = static_cast<float>(i) / step;
            rgbw_t current_color = {
                static_cast<uint8_t>(color_1.r + ratio * (color_2.r - color_1.r)),
                static_cast<uint8_t>(color_1.g + ratio * (color_2.g - color_1.g)),
                static_cast<uint8_t>(color_1.b + ratio * (color_2.b - color_1.b)),
                static_cast<uint8_t>(color_1.w + ratio * (color_2.w - color_1.w))
            };
            // Set LED to current_color
            // Example: setLED(current_color);
            delay(step_interval);
        }
    }
}

void execute_fade(const cmd_fade_t* fade_cmd) {
    fade_led(fade_cmd->color_1, fade_cmd->color_2, fade_cmd->step, fade_cmd->step_interval, fade_cmd->ping_pong);
}

static void handle_fade_command() {
    cmd_fade_t fade_cmd;
    read_command(&fade_cmd);
    execute_fade(&fade_cmd);
}

static void on_receive_command(int len) {
    if(len>0 && len<=max_size) {
        Wire.readBytes(command_buffer,len);
        command = *command_buffer;

        switch (command)
        {
        case CMD_FADE:
          handle_fade_command();
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