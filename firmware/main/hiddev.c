#include <stdint.h>
#include <string.h>
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
#include "nvs.h"
#include "nvs_flash.h"
#include "tinyusb.h"

#define TAG "hiddev"

#define HIDDEV_ACCELEROMETER_FIELDS(type) \
	HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2), \
	HID_USAGE(HIDDEV_USAGE_ACCELEROMETER), \
	HID_COLLECTION(HID_COLLECTION_PHYSICAL), \
		HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
		HID_USAGE(HID_USAGE_DESKTOP_X), \
		HID_USAGE(HID_USAGE_DESKTOP_Y), \
		HID_USAGE(HID_USAGE_DESKTOP_Z), \
		HID_UNIT_N(0xE011, 2), \
		HID_UNIT_EXPONENT(2), \
		HID_LOGICAL_MIN_N(IMU_ACCELEROMETER_LOGICAL_MIN, 3), \
		HID_LOGICAL_MAX_N(IMU_ACCELEROMETER_LOGICAL_MAX, 3), \
		HID_PHYSICAL_MIN_N(IMU_ACCELEROMETER_PHYSICAL_MIN, 3), \
		HID_PHYSICAL_MAX_N(IMU_ACCELEROMETER_PHYSICAL_MAX, 3), \
		HID_REPORT_SIZE(IMU_ACCELEROMETER_LOGICAL_BITS), \
		HID_REPORT_COUNT(3), \
		type(HID_VARIABLE | HID_ABSOLUTE), \
	HID_COLLECTION_END

#define HIDDEV_GYROSCOPE_FIELDS(type) \
	HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2), \
	HID_USAGE(HIDDEV_USAGE_GYROSCOPE), \
	HID_COLLECTION(HID_COLLECTION_PHYSICAL), \
		HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
		HID_USAGE(HID_USAGE_DESKTOP_RX), \
		HID_USAGE(HID_USAGE_DESKTOP_RY), \
		HID_USAGE(HID_USAGE_DESKTOP_RZ), \
		HID_UNIT_N(0xF014, 2), \
		HID_UNIT_EXPONENT(0), \
		HID_LOGICAL_MIN_N(IMU_GYROSCOPE_LOGICAL_MIN, 3), \
		HID_LOGICAL_MAX_N(IMU_GYROSCOPE_LOGICAL_MAX, 3), \
		HID_PHYSICAL_MIN_N(IMU_GYROSCOPE_PHYSICAL_MIN, 3), \
		HID_PHYSICAL_MAX_N(IMU_GYROSCOPE_PHYSICAL_MAX, 3), \
		HID_REPORT_SIZE(IMU_GYROSCOPE_LOGICAL_BITS), \
		HID_REPORT_COUNT(3), \
		type(HID_VARIABLE | HID_ABSOLUTE), \
	HID_COLLECTION_END

#define HIDDEV_CLOCK_FIELDS(type) \
	HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2), \
	HID_USAGE(HIDDEV_USAGE_CLOCK), \
	HID_COLLECTION(HID_COLLECTION_PHYSICAL), \
		HID_USAGE(HIDDEV_USAGE_CLOCK), \
		HID_UNIT_N(0x1001, 2), \
		HID_UNIT_EXPONENT(10), \
		HID_LOGICAL_MIN(0), \
		HID_LOGICAL_MAX_N(IMU_CLOCK_LOGICAL_MAX, 3), \
		HID_PHYSICAL_MIN(0), \
		HID_PHYSICAL_MAX_N(IMU_CLOCK_PHYSICAL_MAX, 3), \
		HID_REPORT_SIZE(IMU_CLOCK_LOGICAL_BITS), \
		HID_REPORT_COUNT(1), \
		type(HID_VARIABLE | HID_ABSOLUTE | HID_WRAP), \
		HID_USAGE(HIDDEV_USAGE_CLOCK), \
		HID_REPORT_COUNT(1), \
		type(HID_VARIABLE | HID_RELATIVE), \
	HID_COLLECTION_END

#define HIDDEV_NVS_NAMESPACE "hiddev"
#define HIDDEV_NVS_KEY_MODE "mode"
#define HIDDEV_NVS_KEY_BIAS "bias"

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
		HIDDEV_ACCELEROMETER_FIELDS(HID_INPUT),
		#endif
		#ifdef IMU_GYROSCOPE_LOGICAL_BITS
		HIDDEV_GYROSCOPE_FIELDS(HID_INPUT),
		#endif
		#ifdef IMU_CLOCK_LOGICAL_BITS
		HIDDEV_CLOCK_FIELDS(HID_INPUT),
		#endif
	HID_COLLECTION_END,
	
	HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2),
	HID_USAGE(HIDDEV_USAGE_CONFIGURE),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(HIDDEV_REPORT_ID_BIAS)
		#ifdef IMU_ACCELEROMETER_LOGICAL_BITS
		HIDDEV_ACCELEROMETER_FIELDS(HID_FEATURE),
		#endif
		#ifdef IMU_GYROSCOPE_LOGICAL_BITS
		HIDDEV_GYROSCOPE_FIELDS(HID_FEATURE),
		#endif
	HID_COLLECTION_END,
	
	HID_USAGE_PAGE_N(HIDDEV_USAGE_PAGE, 2),
	HID_USAGE(HIDDEV_USAGE_CONFIGURE),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(HIDDEV_REPORT_ID_MODE)
		HID_USAGE(HIDDEV_USAGE_MODE),
		HID_LOGICAL_MIN(0),
		HID_LOGICAL_MAX(HIDDEV_MODE_COUNT - 1),
		HID_PHYSICAL_MIN(0),
		HID_PHYSICAL_MAX(0),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(1),
		HID_UNIT(0),
		HID_FEATURE(HID_VARIABLE),
	HID_COLLECTION_END
};

static const uint8_t hiddev_usb_configuration_descriptor[] = {
	TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN, TUSB_REQ_FEATURE_REMOTE_WAKEUP, 100),
	TUD_HID_DESCRIPTOR(0, 4, HID_ITF_PROTOCOL_NONE, sizeof(hiddev_report_descriptor), 0x81, sizeof(hiddev_reportdata_t), 1)
};

static nvs_handle_t hiddev_nvs_handle;
static uint8_t hiddev_mode;
static imu_data_t hiddev_bias;

static const size_t hiddev_bias_report_size =
	#ifdef IMU_CLOCK_LOGICAL_BITS
	sizeof(imu_data_t) - sizeof(imu_clock_t);
	#else
	sizeof(imu_data_t);
	#endif

esp_err_t hiddev_setup() {
	esp_err_t last_error = ESP_OK;
	
	switch (nvs_flash_init()) {
		case ESP_ERR_NVS_NO_FREE_PAGES:
		case ESP_ERR_NVS_NEW_VERSION_FOUND: {
			TRY(last_error, nvs_flash_erase());
			TRY(last_error, nvs_flash_init());
			break;
		}
		case ESP_ERR_NVS_NOT_FOUND: {
			ESP_LOGE(TAG, "NVS partition not found");
			return ESP_FAIL;
		}
	}
	
	TRY(last_error, nvs_open(HIDDEV_NVS_NAMESPACE, NVS_READWRITE, &hiddev_nvs_handle));
	ESP_LOGI(TAG, "Reading saved configuration from flash...");
	
	if ((last_error = nvs_get_u8(hiddev_nvs_handle, HIDDEV_NVS_KEY_MODE, &hiddev_mode)) == ESP_ERR_NVS_NOT_FOUND) {
		hiddev_mode = HIDDEV_MODE_DATA;
		TRY(last_error, nvs_set_u8(hiddev_nvs_handle, HIDDEV_NVS_KEY_MODE, hiddev_mode));
	} else if (last_error != ESP_OK) {
		return last_error;
	}
	
	size_t bias_length;
	if (
		(last_error = nvs_get_blob(hiddev_nvs_handle, HIDDEV_NVS_KEY_BIAS, &hiddev_bias, &bias_length)) == ESP_ERR_NVS_NOT_FOUND ||
		bias_length != sizeof(imu_data_t)
	) {
		memset(&hiddev_bias, 0, sizeof(imu_data_t));
		TRY(last_error, nvs_set_blob(hiddev_nvs_handle, HIDDEV_NVS_KEY_BIAS, &hiddev_bias, sizeof(imu_data_t)));
	} else if (last_error != ESP_OK) {
		return last_error;
	}
	
	ESP_LOGI(TAG, "Obtained configuration from flash, current mode is 0x%02x", hiddev_mode);
	
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
	if (!tud_mounted()) return;
	imu_data_t report;
	memcpy(&report, data, sizeof(imu_data_t));
	
	#ifdef IMU_ACCELEROMETER_LOGICAL_BITS
	report.acc_x -= hiddev_bias.acc_x;
	report.acc_y -= hiddev_bias.acc_y;
	report.acc_z -= hiddev_bias.acc_z;
	#endif
	
	#ifdef IMU_GYROSCOPE_LOGICAL_BITS
	report.gyro_x -= hiddev_bias.gyro_x;
	report.gyro_y -= hiddev_bias.gyro_y;
	report.gyro_z -= hiddev_bias.gyro_z;
	#endif
	
	switch (HIDDEV_MODE_DATA) {
		case HIDDEV_MODE_DATA: {
			tud_hid_report(HIDDEV_REPORT_ID_DATA, &report, sizeof(imu_data_t));
			break;
		}
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
		case HIDDEV_REPORT_ID_MODE: {
			*buffer = hiddev_mode;
			return sizeof(uint8_t);
		}
		case HIDDEV_REPORT_ID_BIAS: {
			memcpy(buffer, &hiddev_bias, hiddev_bias_report_size);
			return hiddev_bias_report_size;
		}
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
		case HIDDEV_REPORT_ID_MODE: {
			if (*buffer >= HIDDEV_MODE_COUNT) return;
			hiddev_mode = *buffer;
			ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_u8(hiddev_nvs_handle, HIDDEV_NVS_KEY_MODE, hiddev_mode));
			ESP_LOGI(TAG, "Current mode is now 0x%02x", hiddev_mode);
			return;
		}
		case HIDDEV_REPORT_ID_BIAS: {
			memcpy(&hiddev_bias, buffer, hiddev_bias_report_size);
			ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_blob(hiddev_nvs_handle, HIDDEV_NVS_KEY_BIAS, &hiddev_bias, sizeof(imu_data_t)));
			ESP_LOGI(TAG, "Updated bias values");
			return;
		}
		default: {
			return;
		}
	}
}
