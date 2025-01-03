#include <Arduino.h>
#include <Wire.h>
#include <esp_ota_ops.h>
#include <WS2812FX.h>
#include "interface.h"
#include "i2c_slave.h"
#include "build.h"

#define LED_PIN 15
#define NUM_LEDS 60

static esp_ota_handle_t ota_update_handle = 0;
static uint8_t* ota_update_data = nullptr;
static size_t ota_update_data_size = 0;
static size_t ota_update_data_total = 0;

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
static uint8_t ota_checksum(const uint8_t* data, size_t data_length,
                        uint8_t seed = 0xFE) {
    uint32_t res = seed;
    while (data_length--) {
        res ^= *(data++);
    }
    return res;
}
static void ota_cancel_update() {
    if(ota_update_handle!=0) {
        ota_update_handle = 0;
    }
    if(ota_update_data!=nullptr) {
        free(ota_update_data);
        ota_update_data = nullptr;
    }
    ota_update_data_size = 0;
    ota_update_data_total = 0;
}
// Command Dispatcher
static void handle_command(uint8_t command, const void* data, size_t data_len) {
    switch (command) {
        case CMD_SOLID:
            ota_cancel_update(); // cancel any pending update
            execute_solid(reinterpret_cast<const cmd_solid_t*>(data));
            break;
        case CMD_BLINK:
            ota_cancel_update(); // cancel any pending update
            execute_blink(reinterpret_cast<const cmd_blink_t*>(data));
            break;
        case CMD_FADE:
            ota_cancel_update(); // cancel any pending update
            execute_fade(reinterpret_cast<const cmd_fade_t*>(data));
            break;
        case CMD_BOUNCE:
            ota_cancel_update(); // cancel any pending update
            execute_bounce(reinterpret_cast<const cmd_bounce_t*>(data));
            break;
        case CMD_BREATH:
            ota_cancel_update(); // cancel any pending update
            execute_breath(reinterpret_cast<const cmd_breath_t*>(data));
            break;
        case CMD_OTA_VER:
            ota_cancel_update(); // cancel any pending update
            // do nothing else here (prevents "unknown command")
            break;
        case CMD_OTA_START: {
                ota_cancel_update(); // cancel any pending update
                Serial.println("Update start");
                const cmd_ota_start_t& start = *reinterpret_cast<const cmd_ota_start_t*>(data);
                ota_update_data= (uint8_t*) ps_malloc(start.size);
                if(ota_update_data == nullptr) {
                    Serial.println("Out of memory performing update.");
                } else {
                    Serial.printf("Allocated %dKB for firmware update\n",(start.size+512)/1024);
                }
                ota_update_data_total = start.size;
                ota_update_data_size = 0;
            }
            break;
        case CMD_OTA_BLOCK: {
                if(ota_update_data==nullptr) {
                    // canceled
                    break;
                }
                const cmd_ota_block_t & block = *reinterpret_cast<const cmd_ota_block_t*>(data);
                //printf("Checksum: %02X vs %02X (master)\n",ota_checksum(block.data,block.length),block.chk);
                if(block.chk != ota_checksum(block.data,block.length)) {
                    Serial.print("Checksum failed for block ");
                    Serial.println(block.seq);
                    ota_cancel_update();
                    break;
                }
                //printf("Update size: %d, Update total: %d, Data length: %d\n",ota_update_data_size,ota_update_data_total,block.length);
                memcpy(ota_update_data+ota_update_data_size,block.data,block.length);
                ota_update_data_size+=block.length;
            }
            break;
        case CMD_OTA_DONE: {
                if(ota_update_data==nullptr) {
                    // canceled
                    break;
                }
                if(ota_update_data_size!=ota_update_data_total) {
                    Serial.println("Update sizes don't match");
                    ota_cancel_update();
                    break;
                }
                if(ESP_OK!=esp_ota_begin(esp_ota_get_next_update_partition(NULL),ota_update_data_total,&ota_update_handle)) {
                    Serial.println("Could not begin firmware write");
                    ota_cancel_update();
                    break;
                }
                ota_update_data_size = 0;
                const uint8_t* p = ota_update_data;
                size_t blocks =0;
                while(ota_update_data_size<ota_update_data_total) {
                    size_t to_write = 8192;
                    if(to_write>ota_update_data_total-ota_update_data_size) {
                        to_write = ota_update_data_total-ota_update_data_size;
                    }
                    if(ESP_OK!=esp_ota_write(ota_update_handle,p,to_write)) {
                        Serial.print("Firmare write failed for block ");
                        Serial.println(blocks);
                        ota_cancel_update();
                        break;
                    }
                    p+=to_write;
                    ota_update_data_size+=to_write;
                    ++blocks;
                }
                
                const cmd_ota_done_t& done = *reinterpret_cast<const cmd_ota_done_t*>(data);
                if(ESP_OK!=esp_ota_end(ota_update_handle)) {
                    Serial.println("Update failed");
                    ota_cancel_update();
                    break;
                }
                ota_update_handle = 0;
                if(ESP_OK!=esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
                    Serial.println("Failed to update boot partition");
                    ota_cancel_update();
                    break;
                }
                Serial.println("Update succeeded. Restarting...");
                ESP.restart();
            }
            break;
        default:
            ota_cancel_update();
            Serial.println("Unknown command");
    }
}

void i2c_command_received(uint8_t cmd, const void* data, size_t data_len) {
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