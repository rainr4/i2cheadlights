#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H
#include <stdint.h>
#include <stddef.h>
#ifndef I2C_SDA
#define I2C_SDA 21
#endif
#ifndef I2C_SCL
#define I2C_SCL 22
#endif
#ifndef I2C_DEV_ADDR
#define I2C_DEV_ADDR 0x40
#endif
#define I2C_BUF_SIZE (256+1)
extern void i2c_initialize();
extern void i2c_command_received(uint8_t cmd, const void* data, size_t data_len);
extern void i2c_request_received(uint8_t cmd);

#endif // I2C_SLAVE_H