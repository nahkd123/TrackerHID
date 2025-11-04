#include <stdint.h>
#include "class/hid/hid_device.h"
#include "common/tusb_types.h"
#include "config.h"
#include "device/usbd.h"
#include "esp_err.h"
#include "esp_log.h"
#include "imu.h"
#include "hiddev.h"
#include "class/hid/hid.h"
#include "imu_bmi160.h"
#include "macros.h"
#include "tinyusb.h"

#define TAG "hiddev"

const char* hiddev_string_descriptor[] = {
	(char[]){ 0x09, 0x04 },
	TRACKER_MANUFACTURER,
	TRACKER_DEVICE_NAME,
	TRACKER_SERIAL_NUMBER,
	TRACKER_INTERFACE_NAME
};

const uint8_t hiddev_report_descriptor[] = {
	HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2),
	HID_USAGE(HIDDEV_USAGE_DATA),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(HIDDEV_REPORT_ID_DATA)
		
		#ifdef IMU_ACCELEROMETER_LOGICAL_BITS
		HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2),
		HID_USAGE(HIDDEV_USAGE_ACCELEROMETER),
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
			HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
			HID_USAGE(HID_USAGE_DESKTOP_X),
			HID_USAGE(HID_USAGE_DESKTOP_Y),
			HID_USAGE(HID_USAGE_DESKTOP_Z),
			HID_UNIT_N(0xE011, 2),
			HID_UNIT_EXPONENT(2),
			HID_LOGICAL_MIN_N(IMU_ACCELEROMETER_LOGICAL_MIN, 3),
			HID_LOGICAL_MAX_N(IMU_ACCELEROMETER_LOGICAL_MAX, 3),
			HID_PHYSICAL_MIN_N(IMU_ACCELEROMETER_PHYSICAL_MIN, 3),
			HID_PHYSICAL_MAX_N(IMU_ACCELEROMETER_PHYSICAL_MAX, 3),
			HID_REPORT_SIZE(IMU_ACCELEROMETER_LOGICAL_BITS),
			HID_REPORT_COUNT(3),
			HID_INPUT(HID_VARIABLE | HID_ABSOLUTE),
		HID_COLLECTION_END,
		#endif
		
		#ifdef IMU_GYROSCOPE_LOGICAL_BITS
		HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2),
		HID_USAGE(HIDDEV_USAGE_GYROSCOPE),
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
			HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
			HID_USAGE(HID_USAGE_DESKTOP_RX),
			HID_USAGE(HID_USAGE_DESKTOP_RY),
			HID_USAGE(HID_USAGE_DESKTOP_RZ),
			HID_UNIT_N(0xF014, 2),
			HID_UNIT_EXPONENT(0),
			HID_LOGICAL_MIN_N(IMU_GYROSCOPE_LOGICAL_MIN, 3),
			HID_LOGICAL_MAX_N(IMU_GYROSCOPE_LOGICAL_MAX, 3),
			HID_PHYSICAL_MIN_N(IMU_GYROSCOPE_PHYSICAL_MIN, 3),
			HID_PHYSICAL_MAX_N(IMU_GYROSCOPE_PHYSICAL_MAX, 3),
			HID_REPORT_SIZE(IMU_GYROSCOPE_LOGICAL_BITS),
			HID_REPORT_COUNT(3),
			HID_INPUT(HID_VARIABLE | HID_ABSOLUTE),
		HID_COLLECTION_END,
		#endif
		
		#ifdef IMU_CLOCK_LOGICAL_BITS
		HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2),
		HID_USAGE(HIDDEV_USAGE_CLOCK),
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
			HID_USAGE(HIDDEV_USAGE_CLOCK),
			HID_UNIT_N(0x1001, 2),
			HID_UNIT_EXPONENT(10),
			HID_LOGICAL_MIN(0),
			HID_LOGICAL_MAX_N(IMU_CLOCK_LOGICAL_MAX, 3),
			HID_PHYSICAL_MIN(0),
			HID_PHYSICAL_MAX_N(IMU_CLOCK_PHYSICAL_MAX, 3),
			HID_REPORT_SIZE(IMU_CLOCK_LOGICAL_BITS),
			HID_REPORT_COUNT(1),
			HID_INPUT(HID_VARIABLE | HID_ABSOLUTE | HID_WRAP),
			HID_USAGE(HIDDEV_USAGE_CLOCK),
			HID_REPORT_COUNT(1),
			HID_INPUT(HID_VARIABLE | HID_RELATIVE),
		HID_COLLECTION_END,
		#endif
	HID_COLLECTION_END,
	
	HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
	HID_USAGE(HID_USAGE_DESKTOP_MOUSE),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(HIDDEV_REPORT_ID_MOUSE)
		HID_USAGE(HID_USAGE_DESKTOP_POINTER),
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
			HID_USAGE(HID_USAGE_DESKTOP_X),
			HID_USAGE(HID_USAGE_DESKTOP_Y),
			HID_LOGICAL_MIN_N(-32768, 2),
			HID_LOGICAL_MAX_N(32767, 2),
			HID_PHYSICAL_MIN(0),
			HID_PHYSICAL_MAX(0),
			HID_REPORT_SIZE(16),
			HID_REPORT_COUNT(2),
			HID_INPUT(HID_VARIABLE | HID_RELATIVE),
		HID_COLLECTION_END,
	HID_COLLECTION_END
};

static const uint8_t hiddev_usb_configuration_descriptor[] = {
	TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN, TUSB_REQ_FEATURE_REMOTE_WAKEUP, 100),
	TUD_HID_DESCRIPTOR(0, 4, HID_ITF_PROTOCOL_NONE, sizeof(hiddev_report_descriptor), 0x81, sizeof(imu_data_t), 1)
};

esp_err_t hiddev_setup() {
	esp_err_t last_error = ESP_OK;
	
	static tusb_desc_device_t tusb_device_desc = {
		.bLength = sizeof(tusb_desc_device_t),
		.bDescriptorType = TUSB_DESC_DEVICE,
		.bcdUSB = 0x0200,
		.bDeviceClass = 0,
		.bDeviceSubClass = 0,
		.bDeviceProtocol = 0,
		.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
		.idVendor = TRACKER_VID,
		.idProduct = TRACKER_PID,
		.bcdDevice = 0x0100,
		.iManufacturer = 1,
		.iProduct = 2,
		.iSerialNumber = 3,
		.bNumConfigurations = 1
	};

	tinyusb_config_t tusb_config = (tinyusb_config_t){
		.port = TINYUSB_PORT_FULL_SPEED_0,
		.phy =
			{
				.skip_setup = 0,
				.self_powered = 0,
				.vbus_monitor_io = -1,
			},
		.task =
			(tinyusb_task_config_t){
				.size = 4096,
				.priority = 5,
				.xCoreID = (1U),
			},
		.descriptor =
			{
				.device = &tusb_device_desc,
				.qualifier = NULL,
				.string = hiddev_string_descriptor,
				.string_count = 6,
				.full_speed_config = hiddev_usb_configuration_descriptor,
				.high_speed_config = hiddev_usb_configuration_descriptor,
			},
		.event_cb = NULL,
		.event_arg = NULL,
	};
	
	TRY(last_error, tinyusb_driver_install(&tusb_config));
	ESP_LOGI(TAG, "Installed TinyUSB on port %d", tusb_config.port);
	return ESP_OK;
}

void hiddev_report_data(const imu_data_t* data) {
	if (tud_mounted()) {
		tud_hid_report(HIDDEV_REPORT_ID_DATA, data, sizeof(imu_data_t));
	}
}

void hiddev_report_mouse(int16_t dx, int16_t dy) {
	if (tud_mounted()) {
		int16_t positions[] = { dx, dy };
		tud_hid_report(HIDDEV_REPORT_ID_MOUSE, positions, sizeof(int16_t) * 2);
	}
}

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
	return hiddev_report_descriptor;
}

uint16_t tud_hid_get_report_cb(
	uint8_t instance,
	uint8_t report_id,
	hid_report_type_t report_type,
	uint8_t *buffer,
	uint16_t reqlen
) {
	switch (report_id) {
		default: {
			return 0;
		}
	}
}

void tud_hid_set_report_cb(
	uint8_t instance,
	uint8_t report_id,
	hid_report_type_t report_type,
	uint8_t const *buffer,
	uint16_t bufsize
) {
	switch (report_id) {
		default: {
			return;
		}
	}
}
