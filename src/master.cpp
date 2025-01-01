#include "Wire.h"
#include <Arduino.h>
#include <stdint.h>
#include <interface.h>

#define I2C_RHL_ADDR 0x40
#define I2C_LHL_ADDR 0x41

constexpr static const int max_size = 1024;
static uint8_t command_buffer[max_size];

// Animation commands
const int animationCommands[] = {
    CMD_SOLID,
    CMD_BLINK,
    CMD_FADE,
    CMD_BOUNCE,
    CMD_BREATH
};

const int numAnimations = sizeof(animationCommands) / sizeof(animationCommands[0]);

//(example values for testing)
rgbw_t defaultColor = {255, 0, 0, 0}; // Red
rgbw_t secondaryColor = {0, 255, 0, 0}; // Green

void sendCommand(uint8_t address, uint8_t command, const void* data, size_t size) {
    if (size > max_size - 1) {
        Serial.println("Data size exceeds buffer limit.");
        return;
    }

    command_buffer[0] = command;
    memcpy(command_buffer + 1, data, size);

    Wire.beginTransmission(address);
    Wire.write(command_buffer, size + 1);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
        Serial.printf("Command %u sent to device 0x%02X\n", command, address);
    } else {
        Serial.printf("Failed to send command %u to device 0x%02X, error: %u\n", command, address, error);
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22, 100 * 1000); // Initialize I2C
    Serial.println("Master ready");
}

void loop() {
    static uint32_t lastAnimationTime = 0;
    static int currentAnimation = 0;

    if (millis() - lastAnimationTime > 5000) { 
        lastAnimationTime = millis();

        uint8_t command = animationCommands[currentAnimation];

        switch (command) {
            case CMD_SOLID: {
                cmd_solid_t solidCommand = {defaultColor};
                sendCommand(I2C_RHL_ADDR, command, &solidCommand, sizeof(solidCommand));
                Serial.println("exec SOLID");
                break;
            }
            case CMD_BLINK: {
                cmd_blink_t blinkCommand = {defaultColor, 500, secondaryColor, 500};
                sendCommand(I2C_RHL_ADDR, command, &blinkCommand, sizeof(blinkCommand));
                Serial.println("exec BLINK");
                break;
            }
            case CMD_FADE: {
                cmd_fade_t fadeCommand = {defaultColor, secondaryColor, 100, 50, true};
                sendCommand(I2C_RHL_ADDR, command, &fadeCommand, sizeof(fadeCommand));
                Serial.println("exec FADE");
                break;
            }
            case CMD_BOUNCE: {
                cmd_bounce_t bounceCommand = {defaultColor, 5, 100, 500};
                sendCommand(I2C_RHL_ADDR, command, &bounceCommand, sizeof(bounceCommand));
                Serial.println("exec BOUNCE");
                break;
            }
            case CMD_BREATH: {
                cmd_breath_t breathCommand = {defaultColor, 2000, 2000, 1000};
                sendCommand(I2C_RHL_ADDR, command, &breathCommand, sizeof(breathCommand));
                Serial.println("exec BREATH");
                break;
            }

            default:
                Serial.println("Unknown command");
                break;
        }

        // Cycle to next animation
        currentAnimation++;
        if (currentAnimation >= sizeof(animationCommands)) {
            currentAnimation = 0;
        }
    }
}
