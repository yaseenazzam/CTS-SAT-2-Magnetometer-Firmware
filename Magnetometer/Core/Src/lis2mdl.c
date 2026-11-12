#include "lis2mdl.h"
#include <stddef.h>

/* STM32 HAL requires the 7-bit address shifted left once. */
#define LIS2MDL_ADDRESS       (0x1EU << 1)
#define LIS2MDL_TIMEOUT_MS    100U

#define REG_WHO_AM_I         0x4FU
#define REG_CFG_A            0x60U
#define REG_CFG_B            0x61U
#define REG_CFG_C            0x62U
#define REG_STATUS           0x67U
#define REG_OUT_X_L          0x68U

static HAL_StatusTypeDef ReadRegister(
    I2C_HandleTypeDef *i2c,
    uint8_t reg,
    uint8_t *buffer,
    uint16_t length)
{
    /* Enable address auto-increment for multiple bytes. */
    if (length > 1U)
    {
        reg |= 0x80U;
    }

    return HAL_I2C_Mem_Read(
        i2c,
        LIS2MDL_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        buffer,
        length,
        LIS2MDL_TIMEOUT_MS);
}

static HAL_StatusTypeDef WriteRegister(
    I2C_HandleTypeDef *i2c,
    uint8_t reg,
    uint8_t value)
{
    return HAL_I2C_Mem_Write(
        i2c,
        LIS2MDL_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1U,
        LIS2MDL_TIMEOUT_MS);
}

HAL_StatusTypeDef LIS2MDL_Init(I2C_HandleTypeDef *i2c)
{
    uint8_t value;
    uint32_t start;
    HAL_StatusTypeDef result;

    if (i2c == NULL)
    {
        return HAL_ERROR;
    }

    HAL_Delay(20);

    /* Confirm that the connected device is a LIS2MDL. */
    result = ReadRegister(i2c, REG_WHO_AM_I, &value, 1U);

    if (result != HAL_OK)
    {
        return result;
    }

    if (value != 0x40U)
    {
        return HAL_ERROR;
    }

    /* Software reset. */
    result = WriteRegister(i2c, REG_CFG_A, 0x20U);

    if (result != HAL_OK)
    {
        return result;
    }

    /* Wait for the reset bit to clear. */
    start = HAL_GetTick();

    while (1)
    {
        HAL_Delay(1);

        result = ReadRegister(i2c, REG_CFG_A, &value, 1U);

        if (result != HAL_OK)
        {
            return result;
        }

        if ((value & 0x20U) == 0U)
        {
            break;
        }

        if ((uint32_t)(HAL_GetTick() - start) >= 100U)
        {
            return HAL_TIMEOUT;
        }
    }

    /* Block data update: keep each output word consistent
       while its two bytes are being read. */
    result = WriteRegister(i2c, REG_CFG_C, 0x10U);

    if (result != HAL_OK)
    {
        return result;
    }

    /* Enable internal offset cancellation. */
    result = WriteRegister(i2c, REG_CFG_B, 0x02U);

    if (result != HAL_OK)
    {
        return result;
    }

    /* Temperature compensation enabled.
       High-resolution mode, 10 Hz, continuous measurement. */
    return WriteRegister(i2c, REG_CFG_A, 0x80U);
}

static int16_t DecodeSigned16(const uint8_t *bytes)
{
    uint16_t value =
        (uint16_t)bytes[0] |
        ((uint16_t)bytes[1] << 8);

    int32_t signed_value = value;

    if ((value & 0x8000U) != 0U)
    {
        signed_value -= 65536;
    }

    return (int16_t)signed_value;
}

HAL_StatusTypeDef LIS2MDL_Read(
    I2C_HandleTypeDef *i2c,
    LIS2MDL_Data *data)
{
    uint8_t status;
    uint8_t bytes[6];
    HAL_StatusTypeDef result;

    if (i2c == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    result = ReadRegister(i2c, REG_STATUS, &status, 1U);

    if (result != HAL_OK)
    {
        return result;
    }

    /* ZYXDA indicates a new measurement is available. */
    if ((status & 0x08U) == 0U)
    {
        return HAL_BUSY;
    }

    result = ReadRegister(i2c, REG_OUT_X_L, bytes, 6U);

    if (result != HAL_OK)
    {
        return result;
    }

    data->x_raw = DecodeSigned16(&bytes[0]);
    data->y_raw = DecodeSigned16(&bytes[2]);
    data->z_raw = DecodeSigned16(&bytes[4]);

    /* Sensitivity: 0.15 microtesla per count. */
    data->x_uT = data->x_raw * 0.15f;
    data->y_uT = data->y_raw * 0.15f;
    data->z_uT = data->z_raw * 0.15f;

    return HAL_OK;
}