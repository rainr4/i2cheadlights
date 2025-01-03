#ifndef INTERFACE_H
#define INTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#define UPDATE_BLOCK_SIZE 128
enum {
    CMD_NONE = 0,
    // send commands
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
    CMD_OTA_START=15,
    CMD_OTA_BLOCK=16,
    CMD_OTA_DONE=17,
    // recv commands
    CMD_OTA_VER = 128
};

typedef struct {
    uint8_t r, g, b, w;
} rgbw_t;

// Dummy stop command (already defined)
typedef struct {
    uint8_t unused; // dummy field
} cmd_stop_t;

// Solid command
typedef struct {
    rgbw_t color;
} cmd_solid_t;

// Blink command
typedef struct {
    rgbw_t color_1;
    uint32_t interval1;
    rgbw_t color_2;
    uint32_t interval2;
} cmd_blink_t;

// Fade command
typedef struct {
    rgbw_t color_1;
    rgbw_t color_2;
    uint32_t step;
    uint32_t step_interval;
    bool ping_pong;
} cmd_fade_t;

// Bounce command
typedef struct {
    rgbw_t color;
    uint8_t bounce_count;      
    uint32_t step_interval;    
    uint32_t pause_duration;   
} cmd_bounce_t;

// Breath command
typedef struct {
    rgbw_t color;
    uint32_t inhale_duration;  
    uint32_t exhale_duration;  
    uint32_t hold_duration;    
} cmd_breath_t;

// Chase command
typedef struct {
    rgbw_t color_1;
    rgbw_t color_2;
    uint8_t length;            
    uint32_t speed;            
} cmd_chase_t;

// Twinkles command
typedef struct {
    rgbw_t base_color;
    rgbw_t sparkle_color;
    uint8_t sparkle_count;     
    uint32_t sparkle_duration; 
} cmd_twinkles_t;

// Edge-to-Center command
typedef struct {
    rgbw_t color;
    uint32_t step_interval;    
} cmd_edgetocenter_t;

// Center-to-Edge command
typedef struct {
    rgbw_t color;
    uint32_t step_interval;    
} cmd_centertoedge_t;

// Flow command
typedef struct {
    rgbw_t start_color;
    rgbw_t end_color;
    uint32_t duration;         
    bool loop;                 
} cmd_flow_t;

// Starlight command
typedef struct {
    rgbw_t color_1;
    rgbw_t color_2;
    uint8_t star_count;        
    uint32_t twinkle_duration; 
} cmd_starlight_t;

// Ramp command
typedef struct {
    rgbw_t color;
    uint32_t ramp_up_duration; 
    uint32_t ramp_down_duration; 
    uint32_t hold_duration;    
} cmd_ramp_t;

typedef struct {
    size_t size;
} cmd_ota_start_t;

typedef struct {
    uint32_t seq;
    uint8_t chk;
    size_t length;
    uint8_t data[UPDATE_BLOCK_SIZE];
} cmd_ota_block_t;

typedef struct {
    size_t blocks;
} cmd_ota_done_t;

typedef struct {
    long version;
} cmd_version_t;

#endif
