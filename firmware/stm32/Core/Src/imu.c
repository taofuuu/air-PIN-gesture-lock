#include "imu.h"

extern I2C_HandleTypeDef hi2c1;

#define MPU_ADDR         (0x68 << 1)  // HAL takes the 7-bit address shifted left; AD0 low
#define I2C_TIMEOUT_MS   100

#define REG_SMPLRT_DIV   0x19
#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_XOUT_H 0x3B
#define REG_PWR_MGMT_1   0x6B
#define REG_WHO_AM_I     0x75

static void write_reg(uint8_t reg, uint8_t value)
{
  HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_MS);
}

uint8_t imu_init(void)
{
  uint8_t id = 0;
  HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, REG_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &id, 1, I2C_TIMEOUT_MS);

  write_reg(REG_PWR_MGMT_1, 0x01);    // clear SLEEP (set at power-up), clock from gyro X PLL
  write_reg(REG_CONFIG, 0x03);        // DLPF ~44 Hz, which makes the internal rate 1 kHz
  write_reg(REG_SMPLRT_DIV, 9);       // 1 kHz / (1 + 9) = 100 Hz
  write_reg(REG_GYRO_CONFIG, 0x08);   // FS_SEL = 1: +-500 deg/s
  write_reg(REG_ACCEL_CONFIG, 0x08);  // AFS_SEL = 1: +-4 g
  return id;
}

int imu_read(imu_sample_t *s)
{
  // ACCEL_XOUT_H onwards: ax, ay, az, temperature, gx, gy, gz, each big-endian
  uint8_t b[14];
  if (HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, REG_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, b, sizeof b,
                       I2C_TIMEOUT_MS) != HAL_OK)
  {
    return -1;
  }
  s->ax = (int16_t)(b[0] << 8 | b[1]);
  s->ay = (int16_t)(b[2] << 8 | b[3]);
  s->az = (int16_t)(b[4] << 8 | b[5]);
  s->gx = (int16_t)(b[8] << 8 | b[9]);
  s->gy = (int16_t)(b[10] << 8 | b[11]);
  s->gz = (int16_t)(b[12] << 8 | b[13]);
  return 0;
}
