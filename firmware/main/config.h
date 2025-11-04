#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "./config_t.h"

/*
 * We would keep Espressif's vendor ID, since we are developing on their
 * platform.
 *
 * Product ID 0x4000..0x4007 is allocated for ESP TinyUSB, where the least
 * significant 3 bits indicates the kind of device. In this case, we need HID.
 *
 * Normally you would replace both VID:PID with your own, but since a VID cost
 * about $6,000 to be registered on USB-IF, it would be better to just rely on
 * Espressif's vendor ID for this moment.
 *
 * TrackerHID will be identified by serial number in applications.
 */
#define TRACKER_VID                   0x303A
#define TRACKER_PID                   0x4004

/*
 * IMPORTANT: The serial number must starts with "TrackerHID" for applications
 * to recognize! This will be used for both USBHID and Bluetooth.
 */
#define TRACKER_SERIAL_NUMBER         "TrackerHID-00000727WYSI"
#define TRACKER_DEVICE_NAME           "TrackerHID"
#define TRACKER_MANUFACTURER          "nahkd123"
#define TRACKER_INTERFACE_NAME        "TrackerHID Interface"

/*
 * IMU communication protocol. Possible values are:
 *
 * - IMU_PROTOCOL_I2C: I2C protocol
 * - IMU_PROTOCOL_SPI: SPI protocol
 */
#define IMU_PROTOCOL                  IMU_PROTOCOL_I2C

// I2C configuration
// Configure the SCL and SDA wires according to your schematic.
#define IMU_I2C_PORT                  I2C_NUM_0
#define IMU_I2C_SCL                   GPIO_NUM_4
#define IMU_I2C_SDA                   GPIO_NUM_5

/*
 * Configure the type of IMU you are using here.
 */
#define IMU_TYPE                      IMU_TYPE_BMI160

// BMI160 configuration               IMU_TYPE_BMI160
/*
 * The I2C address for the BMI160 IMU. The address depends on whether SDO pin is
 * being pulled down or up (connected to GND or VCC).
 *
 * - 0x68 if the SDO pin is connected to GND;
 * - 0x69 if the SDO pin is connected to VCC.
 */
#define IMU_BMI160_I2C_ADDRESS        0x68

#endif /* _CONFIG_H_ */
