#ifndef _IMU_H_
#define _IMU_H_

#include "esp_err.h"
#include "imu_bmi160.h"
#include <stdint.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	#ifdef IMU_ACCELEROMETER_LOGICAL_BITS
	#if IMU_ACCELEROMETER_LOGICAL_BITS == 8
	int8_t acc_x;
	int8_t acc_y;
	int8_t acc_z;
	#elif IMU_ACCELEROMETER_LOGICAL_BITS == 16
	int16_t acc_x;
	int16_t acc_y;
	int16_t acc_z;
	#elif IMU_ACCELEROMETER_LOGICAL_BITS == 32
	int32_t acc_x;
	int32_t acc_y;
	int32_t acc_z;
	#endif
	#endif
	
	#ifdef IMU_GYROSCOPE_LOGICAL_BITS
	#if IMU_GYROSCOPE_LOGICAL_BITS == 8
	int8_t gyro_x;
	int8_t gyro_y;
	int8_t gyro_z;
	#elif IMU_GYROSCOPE_LOGICAL_BITS == 16
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	#elif IMU_GYROSCOPE_LOGICAL_BITS == 32
	int32_t gyro_x;
	int32_t gyro_y;
	int32_t gyro_z;
	#endif
	#endif
	
	#ifdef IMU_CLOCK_LOGICAL_BITS
	#if IMU_CLOCK_LOGICAL_BITS == 8
	uint8_t clock_abs;
	uint8_t clock_delta;
	#elif IMU_CLOCK_LOGICAL_BITS == 16
	uint16_t clock_abs;
	uint16_t clock_delta;
	#elif IMU_CLOCK_LOGICAL_BITS == 32
	uint32_t clock_abs;
	uint32_t clock_delta;
	#endif
	#endif
} imu_data_t;

typedef struct {
	uint8_t report_rate;
} imu_config_t;

esp_err_t imu_setup();
esp_err_t imu_reset();
esp_err_t imu_poll(imu_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* _IMU_H_ */
