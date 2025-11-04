# TrackerHID

Exposing IMU as HID interface.

Originally designed for adding barrel rotation to any drawing tablet, but this
project can also be used for anything involving the IMU, like VR tracking for
example.

## HID report

### `0x01`: Sensor data

The report ID `0x01` exposes everything about the IMU, including:

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
    uint32_t clock_abs; // Report timestamp, wraps back to 0 after 0xFFFFFF with timestep of 39ms
    uint32_t clock_delta; // Delta between previous and current report timestamp
};
```

## `0x80`: HID Mouse

Report ID `0x80` is for gyroscope mouse when user pressed the BOOT button to
switch mode, but you probably don't need it in your apps anyways (and you can't
access it on Windows without Zadig either).
