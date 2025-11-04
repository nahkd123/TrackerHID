#include "driver/gpio.h"
#include "esp_bit_defs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "hiddev.h"
#include "imu.h"
#include "soc/gpio_num.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

void app_main(void) {
	const gpio_config_t btn_config = {
		.pin_bit_mask = BIT64(GPIO_NUM_0),
		.mode = GPIO_MODE_INPUT,
		.intr_type = GPIO_INTR_DISABLE,
		.pull_down_en = false,
		.pull_up_en = true
	};
	
	ESP_ERROR_CHECK(gpio_config(&btn_config));
	ESP_ERROR_CHECK(imu_setup());
	ESP_ERROR_CHECK(imu_reset());
	ESP_ERROR_CHECK(hiddev_setup());
	
	static uint8_t mode = 0;
	static bool last_btn_down = false;
	
    while (true) {
		static imu_data_t data;
		ESP_ERROR_CHECK(imu_poll(&data));
		bool btn_down = !gpio_get_level(GPIO_NUM_0);
		
		switch (mode) {
			case 0: {
				hiddev_report_data(&data);
				break;
			}
			case 1: {
				#ifdef IMU_GYROSCOPE_LOGICAL_BITS
				static int32_t accumulated_x = 0;
				static int32_t accumulated_z = 0;
				accumulated_x += data.gyro_x * data.clock_delta;
				accumulated_z += data.gyro_z * data.clock_delta;
				
				if (abs(accumulated_x) >= 8000 || abs(accumulated_z) >= 8000) {
					hiddev_report_mouse(-accumulated_z / 8000, accumulated_x / 8000);
					accumulated_x = accumulated_x % 8000;
					accumulated_z = accumulated_z % 8000;
				}
				
				#endif
				break;
			}
			default: break;
		}
		
		if (last_btn_down ^ btn_down) {
			last_btn_down = btn_down;
			
			if (btn_down) {
				mode = (mode + 1) % 2;
				ESP_LOGI("main", "Switched to mode %d", mode);
			}
		}
    }
}
