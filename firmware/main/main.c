#include "esp_err.h"
#include "hiddev.h"
#include "imu.h"
#include <stdbool.h>
#include <unistd.h>

void app_main(void) {
	ESP_ERROR_CHECK(imu_setup());
	ESP_ERROR_CHECK(imu_reset());
	ESP_ERROR_CHECK(hiddev_setup());
	
    while (true) {
		static imu_data_t data;
		ESP_ERROR_CHECK(imu_poll(&data));
		hiddev_report_data(&data);
    }
}
