#include <Arduino.h>
#include <Wire.h>
#include "i2c_slave.h"

static uint8_t i2c_receive_buffer[I2C_BUF_SIZE];
static size_t i2c_receive_buffer_size=0;

static uint8_t i2c_last_command() { if(i2c_receive_buffer_size==0) return 0; return *i2c_receive_buffer; }
static void i2c_on_receive(int len) {
    assert(len<=sizeof(i2c_receive_buffer));
    uint8_t* p = i2c_receive_buffer;
    assert(len==Wire.readBytes(i2c_receive_buffer,len));
    i2c_receive_buffer_size = len;
    i2c_command_received(i2c_receive_buffer[0],i2c_receive_buffer+1, i2c_receive_buffer_size-1);
}
static void i2c_on_request() {
    uint8_t cmd = i2c_last_command();
    if(cmd!=0) {
        i2c_request_received(cmd);
    }
}
void i2c_initialize(uint8_t addr, uint8_t sda, uint8_t scl, uint32_t freq) {
    Wire.onReceive(i2c_on_receive);
    Wire.onRequest(i2c_on_request);
    Wire.begin(addr, sda,scl,freq);
    Wire.setBufferSize(I2C_BUF_SIZE); 
}