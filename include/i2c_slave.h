#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H
#include <stdint.h>
#include <stddef.h>
#define I2C_BUF_SIZE (256+1)
extern void i2c_initialize(uint8_t addr, uint8_t sda, uint8_t scl, uint32_t freq);
extern void i2c_command_received(uint8_t cmd, const void* data, size_t data_len);
extern void i2c_request_received(uint8_t cmd);

#endif // I2C_SLAVE_H