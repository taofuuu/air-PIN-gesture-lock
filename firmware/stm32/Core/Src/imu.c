#include "imu.h"

extern I2C_HandleTypeDef hi2c1;

#define MPU_ADDR         (0x68 << 1)  // HAL takes the 7-bit address shifted left; AD0 low
#define I2C_TIMEOUT_MS   10           // a 14-byte read at 100 kHz takes ~1.6 ms; stay under one sample period
#define I2C_SCL_PIN      GPIO_PIN_8   // PB8
#define I2C_SDA_PIN      GPIO_PIN_9   // PB9

#define REG_SMPLRT_DIV   0x19
#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_XOUT_H 0x3B
#define REG_PWR_MGMT_1   0x6B
#define REG_WHO_AM_I     0x75

static HAL_StatusTypeDef write_reg(uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_MS);
}

static void bus_line(uint16_t pin, GPIO_PinState level)
{
  HAL_GPIO_WritePin(GPIOB, pin, level);
  HAL_Delay(1);
}

// A chip reset during a read leaves the sensor waiting to send the rest of its byte. A 0 bit
// holds SDA low until a power cycle; a 1 bit looks like an idle bus, and the next transfer
// collides with it. SDA can't tell these apart, so always finish the read by hand.
static void i2c_bus_recover(void)
{
  HAL_I2C_DeInit(&hi2c1);
  HAL_GPIO_WritePin(GPIOB, I2C_SCL_PIN | I2C_SDA_PIN, GPIO_PIN_SET);
  GPIO_InitTypeDef pins = {.Pin = I2C_SCL_PIN | I2C_SDA_PIN, .Mode = GPIO_MODE_OUTPUT_OD};
  HAL_GPIO_Init(GPIOB, &pins);

  // 9 clocks with SDA released always cross one ACK slot, and no ACK there ends a read. A STOP
  // fails if its own clock lands on the sensor's ACK to a write, but ACK slots are 9 clocks
  // apart and these rounds are 10, so the two STOPs can't both fail.
  for (int round = 0; round < 2; round++)
  {
    for (int i = 0; i < 9; i++)
    {
      bus_line(I2C_SCL_PIN, GPIO_PIN_RESET);
      bus_line(I2C_SCL_PIN, GPIO_PIN_SET);
    }

    // STOP: SDA rises while SCL is high
    bus_line(I2C_SCL_PIN, GPIO_PIN_RESET);
    bus_line(I2C_SDA_PIN, GPIO_PIN_RESET);
    bus_line(I2C_SCL_PIN, GPIO_PIN_SET);
    bus_line(I2C_SDA_PIN, GPIO_PIN_SET);
  }

  HAL_I2C_Init(&hi2c1);
}

uint8_t imu_init(void)
{
  uint8_t id = 0;
  i2c_bus_recover();
  HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, REG_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &id, 1, I2C_TIMEOUT_MS);

  // A lost write leaves that register at its power-up default (e.g. +-2 g instead of +-4 g),
  // which would silently record at the wrong scale, so treat any failure as "no sensor".
  if (write_reg(REG_PWR_MGMT_1, 0x01) != HAL_OK) return 0;    // clear SLEEP, clock from gyro X PLL
  if (write_reg(REG_CONFIG, 0x03) != HAL_OK) return 0;        // DLPF ~44 Hz, internal rate 1 kHz
  if (write_reg(REG_SMPLRT_DIV, 9) != HAL_OK) return 0;       // 1 kHz / (1 + 9) = 100 Hz
  if (write_reg(REG_GYRO_CONFIG, 0x08) != HAL_OK) return 0;   // FS_SEL = 1: +-500 deg/s
  if (write_reg(REG_ACCEL_CONFIG, 0x08) != HAL_OK) return 0;  // AFS_SEL = 1: +-4 g
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
