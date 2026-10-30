/* MPU-6050 driver on I2C1 (PB8 = SCL, PB9 = SDA).
 *
 * Fixed settings: 100 Hz sample rate, ~44 Hz low-pass filter, accel +-4 g (8192 counts per g),
 * gyro +-500 deg/s (65.5 counts per deg/s). Training data must be recorded with the same
 * settings, so changing them means re-recording. */
#ifndef IMU_H
#define IMU_H

#include "main.h"

typedef struct { int16_t ax, ay, az, gx, gy, gz; } imu_sample_t;  // raw MPU-6050 counts

/* Wakes the chip and applies the settings above. Returns WHO_AM_I: 0x68 on a genuine
 * MPU-6050, 0x00 if nothing answered on the bus. */
uint8_t imu_init(void);

/* Reads one sample. Returns 0 on success, -1 if the I2C transfer failed. */
int imu_read(imu_sample_t *s);

#endif /* IMU_H */
