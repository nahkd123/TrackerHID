#include "config.h"
#include "imu.h"
#include "macros.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>

#define TAG "IMU"

#if IMU_PROTOCOL == IMU_PROTOCOL_I2C
#include "driver/i2c.h"
#include "hal/i2c_types.h"
#endif

#if IMU_TYPE == IMU_TYPE_BMI160

typedef struct {
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	int16_t acc_x;
	int16_t acc_y;
	int16_t acc_z;
} bmi160_data_t;

static uint32_t bmi160_last_sensortime = 0;

esp_err_t bmi160_write(const uint8_t reg, uint8_t data) {
	const uint8_t bytes[] = { reg, data };
	return i2c_master_write_to_device(IMU_I2C_PORT, IMU_BMI160_I2C_ADDRESS, bytes, 2, -1);
}

esp_err_t bmi160_read(const uint8_t reg, uint8_t* data) {
	return i2c_master_write_read_device(IMU_I2C_PORT, IMU_BMI160_I2C_ADDRESS, &reg, 1, data, 1, -1);
}

esp_err_t imu_setup() {
	esp_err_t last_error = ESP_OK;
	
	#if IMU_PROTOCOL == IMU_PROTOCOL_I2C
	const i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.scl_io_num = IMU_I2C_SCL,
		.sda_io_num = IMU_I2C_SDA,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 1000000
	};
	
	TRY(last_error, i2c_param_config(IMU_I2C_PORT, &i2c_config));
	TRY(last_error, i2c_driver_install(IMU_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));
	ESP_LOGI(TAG, "Installed I2C driver on port %d (SCL %d / SDA %d)", IMU_I2C_PORT, IMU_I2C_SCL, IMU_I2C_SDA);
	return ESP_OK;
	#else
	ESP_LOGI(TAG, "Invalid IMU protocol configured in config.h");
	return ESP_FAIL;
	#endif
}

esp_err_t imu_reset() {
	esp_err_t last_error = ESP_OK;
	
	#if IMU_PROTOCOL == IMU_PROTOCOL_I2C
	uint8_t chip_id = 0xFF;
	
	TRY(last_error, bmi160_write(0x7E, 0xB6));
	vTaskDelay(pdMS_TO_TICKS(100));
	ESP_LOGI(TAG, "Soft reseted IMU");
	
	TRY(last_error, bmi160_read(0x00, &chip_id));
	vTaskDelay(pdMS_TO_TICKS(100));
	ESP_LOGI(TAG, "Chip ID is 0x%02x", chip_id);
	if (chip_id != 0xD1) ESP_LOGW(TAG, "Chip ID is unexpected (must be 0xD1 after reset)");
	
	TRY(last_error, bmi160_write(0x7E, 0x11));
	vTaskDelay(pdMS_TO_TICKS(100));
	ESP_LOGI(TAG, "Enabled accelerometer");
	
	TRY(last_error, bmi160_write(0x7E, 0x15));
	vTaskDelay(pdMS_TO_TICKS(100));
	ESP_LOGI(TAG, "Enabled gyroscope");
	
	TRY(last_error, bmi160_write(0x40, 0x0C)); // 1600Hz output rate
	TRY(last_error, bmi160_write(0x41, 0x0C)); // +- 16 m2/s
	ESP_LOGI(TAG, "Configured accelerometer");
	
	TRY(last_error, bmi160_write(0x42, 0x0C)); // 1600Hz output rate
	TRY(last_error, bmi160_write(0x43, 0x00)); // +- 2000 deg/s
	ESP_LOGI(TAG, "Configured gyroscope");
	
	static const uint8_t reg_sensortime = 0x18;
	TRY(last_error, i2c_master_write_read_device(IMU_I2C_PORT, IMU_BMI160_I2C_ADDRESS, &reg_sensortime, 1, (uint8_t*)&bmi160_last_sensortime, 3, -1));
	#endif
	
	return ESP_OK;
}

esp_err_t imu_poll(imu_data_t* data) {
	esp_err_t last_error = ESP_OK;
	
	#if IMU_PROTOCOL == IMU_PROTOCOL_I2C
	static const uint8_t reg_data = 0x0C;
	static const uint8_t reg_sensortime = 0x18;
	bmi160_data_t bmi160_data;
	uint32_t sensortime = 0;
	
	TRY(last_error, i2c_master_write_read_device(IMU_I2C_PORT, IMU_BMI160_I2C_ADDRESS, &reg_data, 1, (uint8_t*)&bmi160_data, 12, -1));
	TRY(last_error, i2c_master_write_read_device(IMU_I2C_PORT, IMU_BMI160_I2C_ADDRESS, &reg_sensortime, 1, (uint8_t*)&sensortime, 3, -1));
	
	memcpy(&(data->acc_x), &(bmi160_data.acc_x), 6);
	memcpy(&(data->gyro_x), &(bmi160_data.gyro_x), 6);
	data->clock_abs = sensortime;
	data->clock_delta = sensortime > bmi160_last_sensortime ? sensortime - bmi160_last_sensortime : 0xFFFFFF - bmi160_last_sensortime + sensortime;
	
	bmi160_last_sensortime = sensortime;
	#endif
	
	return ESP_OK;
}

#endif