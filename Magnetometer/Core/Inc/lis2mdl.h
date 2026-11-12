#ifndef LIS2MDL_H
#define LIS2MDL_H

#include "stm32l4xx_hal.h"

typedef struct
{
    int16_t x_raw;
    int16_t y_raw;
    int16_t z_raw;

    float x_uT;
    float y_uT;
    float z_uT;
} LIS2MDL_Data;

/* Initialize continuous measurements at 10 Hz.
   Call after the generated I2C initialization. */
HAL_StatusTypeDef LIS2MDL_Init(I2C_HandleTypeDef *i2c);

/* Read a fresh measurement.
   HAL_OK: success
   HAL_BUSY: no new measurement yet
   HAL_ERROR / HAL_TIMEOUT: communication or argument error */
HAL_StatusTypeDef LIS2MDL_Read(I2C_HandleTypeDef *i2c,
                             LIS2MDL_Data *data);

#endif