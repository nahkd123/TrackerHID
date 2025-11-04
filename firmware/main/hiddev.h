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
#define HIDDEV_USAGE_REPORT_RATE   0x25

#define HIDDEV_REPORT_ID_DATA      0x01
#define HIDDEV_REPORT_ID_CONFIGURE 0x02
#define HIDDEV_REPORT_ID_MOUSE     0x80

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hiddev_setup();
void hiddev_report_data(const imu_data_t* data);
void hiddev_report_mouse(int16_t dx, int16_t dy);

#ifdef __cplusplus
}
#endif

#endif /* _HIDDEV_H_ */
