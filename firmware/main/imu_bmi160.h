#ifndef _IMU_BMI160_H_
#define _IMU_BMI160_H_

#include "config.h"

#if IMU_TYPE == IMU_TYPE_BMI160

#define IMU_ACCELEROMETER_LOGICAL_BITS           16
#define IMU_ACCELEROMETER_LOGICAL_MIN        -32768
#define IMU_ACCELEROMETER_LOGICAL_MAX         32767
#define IMU_ACCELEROMETER_PHYSICAL_MIN          -16
#define IMU_ACCELEROMETER_PHYSICAL_MAX           16

#define IMU_GYROSCOPE_LOGICAL_BITS               16
#define IMU_GYROSCOPE_LOGICAL_MIN            -32768
#define IMU_GYROSCOPE_LOGICAL_MAX             32767
#define IMU_GYROSCOPE_PHYSICAL_MIN            -2000
#define IMU_GYROSCOPE_PHYSICAL_MAX             2000

#define IMU_CLOCK_LOGICAL_BITS                   32
#define IMU_CLOCK_LOGICAL_MAX              0xFFFFFF
#define IMU_CLOCK_PHYSICAL_MAX            654311385

#endif
#endif /* _IMU_BMI160_H_ */
