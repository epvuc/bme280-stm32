#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "bme280.h"

extern I2C_HandleTypeDef hi2c1;

int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev);
int8_t stream_sensor_data_normal_mode(struct bme280_dev *dev);
void user_delay_ms(uint32_t ms);

struct bme280_dev bme280;

void my_print_readings(void)
{
  int8_t rslt = BME280_OK;
  bme280.dev_id = BME280_I2C_ADDR_SEC;
  bme280.intf = BME280_I2C_INTF;
  bme280.read = (void *)user_i2c_read;
  bme280.write = (void *)user_i2c_write;
  bme280.delay_ms = (void *)user_delay_ms;
  rslt = bme280_init(&bme280);
  printf("init result %d\r\n", rslt);
  HAL_Delay(100);
  printf("starting readings.\r\n");
  stream_sensor_data_forced_mode(&bme280);
}

void print_sensor_data(struct bme280_data *comp_data)
{
    #ifdef BME280_FLOAT_ENABLE
  printf("%0.2f, %0.2f, %0.2f\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
    #else
  printf("%ld, %ld, %ld\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
    #endif
}

int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev)
{
  int8_t rslt;
  uint8_t settings_sel;
  struct bme280_data comp_data;

  dev->settings.osr_h = BME280_OVERSAMPLING_2X;
  dev->settings.osr_p = BME280_OVERSAMPLING_4X;
  dev->settings.osr_t = BME280_OVERSAMPLING_4X;
  dev->settings.filter = BME280_FILTER_COEFF_2;

  settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

  rslt = bme280_set_sensor_settings(settings_sel, dev);

  printf("Temperature, Pressure, Humidity\r\n");
  /* Continuously stream sensor data */
  while (1) {
    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
    /* Wait for the measurement to complete and print data @25Hz */
    dev->delay_ms(40);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
    print_sensor_data(&comp_data);
    dev->delay_ms(1000);
  }
  return rslt;
}

int8_t stream_sensor_data_normal_mode(struct bme280_dev *dev)
{
  int8_t rslt;
  uint8_t settings_sel;
  struct bme280_data comp_data;

  /* Recommended mode of operation: Indoor navigation */
  dev->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev->settings.osr_p = BME280_OVERSAMPLING_1X;
  dev->settings.osr_t = BME280_OVERSAMPLING_1X;
  dev->settings.filter = BME280_FILTER_COEFF_16;
  dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

  settings_sel = BME280_OSR_PRESS_SEL;
  settings_sel |= BME280_OSR_TEMP_SEL;
  settings_sel |= BME280_OSR_HUM_SEL;
  settings_sel |= BME280_STANDBY_SEL;
  settings_sel |= BME280_FILTER_SEL;
  rslt = bme280_set_sensor_settings(settings_sel, dev);
  rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

  printf("Temperature, Pressure, Humidity\r\n");
  while (1) {
    /* Delay while the sensor completes a measurement */
    dev->delay_ms(140);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
    print_sensor_data(&comp_data);
  }

  return rslt;
}

void user_delay_ms(uint32_t ms)
{
  HAL_Delay(ms);
}

int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
  HAL_StatusTypeDef status = HAL_OK;  

  status = HAL_I2C_Mem_Write(&hi2c1, dev_addr<<1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, cnt, 0xffff);

  if (status == HAL_OK)
    return(0);
  else
    return(-1);
}

int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Read(&hi2c1, dev_addr<<1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, cnt, 0xffff);

  if (status == HAL_OK) 
    return(0);
  else
    return(-1);
} 

