#ifndef GRIDEYE_I2C_H
#define GRIDEYE_I2C_H

#include <stdint.h>
#include <stdbool.h>

// Define the missing ESP-IDF types so the compiler stops crying
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1

// Just the bare minimum function signatures
esp_err_t i2c_master_init(void);
esp_err_t i2c_master_read_slave(uint8_t dev_addr, uint8_t i2c_reg, uint8_t *data_rd, uint8_t size);
esp_err_t i2c_master_write_slave(uint8_t dev_addr, uint8_t i2c_reg, uint8_t *data_wr, uint8_t size);

#endif