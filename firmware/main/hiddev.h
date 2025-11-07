#ifndef _HIDDEV_H_
#define _HIDDEV_H_

#include "esp_err.h"
#include "imu.h"

#define HIDDEV_USAGE_PAGE          0xFF00
#define HIDDEV_USAGE_DATA          0x01
#define HIDDEV_USAGE_CONFIGURE     0x02
#define HIDDEV_USAGE_ACCELEROMETER 0x20
#define HIDDEV_USAGE_GYROSCOPE     0x21
#define HIDDEV_USAGE_COMPASS       0x22
#define HIDDEV_USAGE_TEMPERATURE   0x23
#define HIDDEV_USAGE_CLOCK         0x24
#define HIDDEV_USAGE_MODE          0x25

#define HIDDEV_REPORT_ID_DATA      0x01 /* Raw sensor data */
#define HIDDEV_REPORT_ID_BIAS      0x02 /* Sensor offset calibration */
#define HIDDEV_REPORT_ID_MODE      0x03 /* Change mode */
#define HIDDEV_REPORT_ID_MOUSE     0x80 /* HID mouse (remains unused for now) */

#ifdef __cplusplus
extern "C" {
#endif

enum {
	HIDDEV_MODE_DATA = 0,
	HIDDEV_MODE_COUNT
};

// Union for computing max number of bytes for HID reports
typedef union {
	imu_data_t data_report;
	imu_data_t bias_report;
	uint8_t mode_report;
} hiddev_reportdata_t;

esp_err_t hiddev_setup();
void hiddev_report_data(const imu_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* _HIDDEV_H_ */
