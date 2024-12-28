#ifndef INTERFACE_H
#define INTERFACE_H
#include <stdint.h>

enum {
    CMD_SOLID = 1,
    CMD_BLINK = 2,
    CMD_FADE = 3,
    CMD_BOUNCE = 4,
    CMD_BREATH = 5,
    CMD_CHASE = 6,
    CMD_TWINKLES = 7,
    CMD_EDGETOCENTER = 8,
    CMD_CENTERTOEDGE = 9,
    CMD_FLOW = 10,
    CMD_STARLIGHT = 11,
    CMD_RAMP = 12,
    CMD_DEFAULT = 13,
    CMD_OFF = 14,
};
typedef struct {
    uint8_t r,g,b,w;
} rgbw_t;

typedef struct {
    uint8_t unused; // dummy field
} cmd_stop_t;

typedef struct {
    rgbw_t color;
} cmd_solid_t;

typedef struct {
    rgbw_t color1;
    uint32_t interval1;
    rgbw_t color2;
    uint32_t interval2;
} cmd_blink_t;

typedef struct {
    rgbw_t color1;
    rgbw_t color2;
    uint32_t step;
    uint32_t step_interval;
    bool ping_pong;
} cmd_fade_t;

#endif
