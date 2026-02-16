#include "grideye_i2c.h"

esp_err_t i2c_master_init(void) {
    return ESP_OK;
}

esp_err_t i2c_master_read_slave(uint8_t dev_addr, uint8_t i2c_reg, uint8_t *data_rd, uint8_t size) {
    // We will fill data_rd with dummy data later if we want to test math
    return ESP_OK;
}

esp_err_t i2c_master_write_slave(uint8_t dev_addr, uint8_t i2c_reg, uint8_t *data_wr, uint8_t size) {
    return ESP_OK;
}