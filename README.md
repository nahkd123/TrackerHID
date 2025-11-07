# TrackerHID

Exposing IMU as HID interface.

Originally designed for adding barrel rotation to any drawing tablet, but this
project can also be used for anything involving the IMU, like VR tracking for
example.

## HID report

### `IN 0x01`: Sensor data

The report ID `0x01` exposes (almost) everything about the IMU, including:

- Accelerometer;
- Gyroscope;
- IMU clock (a.k.a hardware timer).

```c
struct {
    // uint8_t report_id;
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    uint32_t clock_abs; // Report timestamp, wraps back to 0 after 0xFFFFFF with timestep of 39 us
    uint32_t clock_delta; // Delta between previous and current report timestamp
};
```

This input report is only available in data mode and the data is adjusted by
some amount configured in "Sensor bias data" feature report. To obtain raw data,
set all bias values to 0.

### `FEATURE 0x02`: Sensor bias data

This feature report is for offsetting the values in sensor data. Initially, this
feature is filled with 0s and the data will be stored in ESP32's flash when
setting this feature report.

```c
struct {
    int16_t bias_acc_x;
    int16_t bias_acc_y;
    int16_t bias_acc_z;
    int16_t bias_gyro_x;
    int16_t bias_gyro_y;
    int16_t bias_gyro_z;
};
```

Bias applies to all modes.

### `FEATURE 0x03`: Mode

This feature report is for setting the output mode of TrackerHID. More modes may
be introduced in the future (when there is demands, I guess).

```c
enum {
	HIDDEV_MODE_DATA = 0
};
```

Just like `0x02`, the mode will be stored in flash when setting the feature
report.

### `IN 0x80`: ~~HID Mouse~~ (Unused)

Report ID `0x80` is for gyroscope mouse when user pressed the BOOT button to
switch mode, but you probably don't need it in your apps anyways (and you can't
access it on Windows without Zadig either).
