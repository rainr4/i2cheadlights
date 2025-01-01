#include "Wire.h"
#include <Arduino.h>
#include <WS2812FX.h>
#include <interface.h> 

#define LED_PIN 15
#define NUM_LEDS 60

// LED strip object
WS2812FX ws2812fx = WS2812FX(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Command buffer and state
constexpr static const int max_size = 1024;
static uint8_t command_buffer[max_size];
static int command = 0; 

template <typename T>
static void read_command(T* out_cmd) {
    memcpy(out_cmd, command_buffer + 1, sizeof(T));
}

// Animation Handlers
void execute_solid(const cmd_solid_t* solid_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(solid_cmd->color.r, solid_cmd->color.g, solid_cmd->color.b, solid_cmd->color.w));
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.start();
}
 
void execute_blink(const cmd_blink_t* blink_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(blink_cmd->color_1.r, blink_cmd->color_1.g, blink_cmd->color_1.b, blink_cmd->color_1.w));
    ws2812fx.setMode(FX_MODE_BLINK);
    ws2812fx.setSpeed(blink_cmd->interval1);
    ws2812fx.start();
}

void execute_fade(const cmd_fade_t* fade_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(fade_cmd->color_1.r, fade_cmd->color_1.g, fade_cmd->color_1.b, fade_cmd->color_1.w));
    ws2812fx.setMode(FX_MODE_FADE);
    ws2812fx.setSpeed(fade_cmd->step_interval);
    ws2812fx.start();
}

void execute_bounce(const cmd_bounce_t* bounce_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(bounce_cmd->color.r, bounce_cmd->color.g, bounce_cmd->color.b, bounce_cmd->color.w));
    ws2812fx.setMode(FX_MODE_SCAN);  // Using scan mode for bounce effect
    ws2812fx.setSpeed(bounce_cmd->step_interval);
    ws2812fx.start();
}

void execute_breath(const cmd_breath_t* breath_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(breath_cmd->color.r, breath_cmd->color.g, breath_cmd->color.b, breath_cmd->color.w));
    ws2812fx.setMode(FX_MODE_BREATH);
    ws2812fx.setSpeed(breath_cmd->inhale_duration);
    ws2812fx.start();
}

// Command Dispatcher
static void handle_command() {
    switch (command) {
        case CMD_SOLID:
            execute_solid(reinterpret_cast<cmd_solid_t*>(command_buffer + 1));
            break;
        case CMD_BLINK:
            execute_blink(reinterpret_cast<cmd_blink_t*>(command_buffer + 1));
            break;
        case CMD_FADE:
            execute_fade(reinterpret_cast<cmd_fade_t*>(command_buffer + 1));
            break;
        default:
            Serial.println("Unknown command");
    }
}

// I2C Receive Handler
static void on_receive_command(int len) {
    if (len > 0 && len <= max_size) {
        Wire.readBytes(command_buffer, len);
        command = *command_buffer;

        Serial.print("Received command: ");
        Serial.println(command);
        handle_command();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Slave ready");

    Wire.begin(I2C_DEV_ADDR);
    Wire.onReceive(on_receive_command);

    ws2812fx.init();
    ws2812fx.setBrightness(128);
    ws2812fx.setColor(ws2812fx.Color(60, 20, 0, 128)); //Default color (White)
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.start();
} 

void loop() {
    ws2812fx.service();
}

//  {255, 180, 100, 0},   // More orange glow
//  {60,  20,  0,   200}, // Mostly white with a hint of red/green
//  {80,  40,  0,   180}, // Another variation