#include <Arduino.h>
#include <Wire.h>
#include <WS2812FX.h>
#include "interface.h"
#include "i2c_slave.h"
#include "build.h"
#define LED_PIN 15
#define NUM_LEDS 60

// LED strip object
WS2812FX ws2812fx = WS2812FX(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Animation Handlers
static void execute_solid(const cmd_solid_t* solid_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(solid_cmd->color.r, solid_cmd->color.g, solid_cmd->color.b, solid_cmd->color.w));
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.start();
}
 
static void execute_blink(const cmd_blink_t* blink_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(blink_cmd->color_1.r, blink_cmd->color_1.g, blink_cmd->color_1.b, blink_cmd->color_1.w));
    ws2812fx.setMode(FX_MODE_BLINK);
    ws2812fx.setSpeed(blink_cmd->interval1);
    ws2812fx.start();
}

static void execute_fade(const cmd_fade_t* fade_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(fade_cmd->color_1.r, fade_cmd->color_1.g, fade_cmd->color_1.b, fade_cmd->color_1.w));
    ws2812fx.setMode(FX_MODE_FADE);
    ws2812fx.setSpeed(fade_cmd->step_interval);
    ws2812fx.start();
}

static void execute_bounce(const cmd_bounce_t* bounce_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(bounce_cmd->color.r, bounce_cmd->color.g, bounce_cmd->color.b, bounce_cmd->color.w));
    ws2812fx.setMode(FX_MODE_SCAN);  // Using scan mode for bounce effect
    ws2812fx.setSpeed(bounce_cmd->step_interval);
    ws2812fx.start();
}

static void execute_breath(const cmd_breath_t* breath_cmd) {
    ws2812fx.stop();
    ws2812fx.setColor(ws2812fx.Color(breath_cmd->color.r, breath_cmd->color.g, breath_cmd->color.b, breath_cmd->color.w));
    ws2812fx.setMode(FX_MODE_BREATH);
    ws2812fx.setSpeed(breath_cmd->inhale_duration);
    ws2812fx.start();
}

// Command Dispatcher
static void handle_command(uint8_t command, const void* data, size_t data_len) {
    switch (command) {
        case CMD_SOLID:
            execute_solid(reinterpret_cast<const cmd_solid_t*>(data));
            break;
        case CMD_BLINK:
            execute_blink(reinterpret_cast<const cmd_blink_t*>(data));
            break;
        case CMD_FADE:
            execute_fade(reinterpret_cast<const cmd_fade_t*>(data));
            break;
        case CMD_BOUNCE:
            execute_bounce(reinterpret_cast<const cmd_bounce_t*>(data));
            break;
        case CMD_BREATH:
            execute_breath(reinterpret_cast<const cmd_breath_t*>(data));
            break;
        case CMD_OTA_VER:
            break;
        default:
            Serial.println("Unknown command");
    }
}

void i2c_command_received(uint8_t cmd, const void* data, size_t data_len) {
    Serial.print("Received command: ");
    Serial.println(cmd);
    handle_command(cmd,data,data_len);
}
void i2c_request_received(uint8_t cmd) {
    Serial.print("Request received: ");
    Serial.println(cmd);
    if(cmd==CMD_OTA_VER) {
        long btime = build_time();
        Serial.printf("Report build id: %08lx\n",btime);
        Wire.slaveWrite((uint8_t*)&btime,sizeof(long));
    }
}
// I2C Receive Handler

void setup() {
    Serial.begin(115200);
    Serial.printf("Build ID: %08lx\n",(long)build_time());
    Serial.println("Slave ready");
    i2c_initialize();
    
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