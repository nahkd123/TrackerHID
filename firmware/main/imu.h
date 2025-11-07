#ifndef _IMU_H_
#define _IMU_H_

#include "esp_err.h"
#include "imu_bmi160.h"
#include <stdint.h>
#include <sys/cdefs.h>

#ifdef IMU_ACCELEROMETER_LOGICAL_BITS
#if IMU_ACCELEROMETER_LOGICAL_BITS == 8
typedef int8_t imu_acc_t;
#elif IMU_ACCELEROMETER_LOGICAL_BITS == 16
typedef int16_t imu_acc_t;
#elif IMU_ACCELEROMETER_LOGICAL_BITS == 32
typedef int32_t imu_acc_t;
#endif
#endif

#ifdef IMU_GYROSCOPE_LOGICAL_BITS
#if IMU_GYROSCOPE_LOGICAL_BITS == 8
typedef int8_t imu_gyro_t;
#elif IMU_GYROSCOPE_LOGICAL_BITS == 16
typedef int16_t imu_gyro_t;
#elif IMU_GYROSCOPE_LOGICAL_BITS == 32
typedef int32_t imu_gyro_t;
#endif
#endif

#ifdef IMU_CLOCK_LOGICAL_BITS
#if IMU_CLOCK_LOGICAL_BITS == 8
typedef uint8_t imu_clock_t;
#elif IMU_CLOCK_LOGICAL_BITS == 16
typedef uint16_t imu_clock_t;
#elif IMU_CLOCK_LOGICAL_BITS == 32
typedef uint32_t imu_clock_t;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	#ifdef IMU_ACCELEROMETER_LOGICAL_BITS
	imu_acc_t acc_x;
	imu_acc_t acc_y;
	imu_acc_t acc_z;
	#endif
	
	#ifdef IMU_GYROSCOPE_LOGICAL_BITS
	imu_gyro_t gyro_x;
	imu_gyro_t gyro_y;
	imu_gyro_t gyro_z;
	#endif
	
	#ifdef IMU_CLOCK_LOGICAL_BITS
	imu_clock_t clock_abs;
	imu_clock_t clock_delta;
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
