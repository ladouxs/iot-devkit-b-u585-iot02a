/*
 * ADS1115.c
 *
 *      Author: e.hollier-larousse
 */
#include "stm32u5xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

// Thermistor constants
#define R_FIXED 47000.0f         // Fixed resistance (ohms)
#define BETA 4090.0f             // β coefficient
#define R0 47000.0f              // Nominal resistance at 25°C
#define T0 298.15f               // Temperature 25°C in Kelvin
#define V_REF 3.3f               // Bridge power supply
#define ADS1115_ADDR (0x48 << 1) // I2C address of the ADS1115 module (<<1 because HAL uses 8 bits)

// Reads voltage from ADS1115 via I2C and converts raw data to volts : () -> float
float ReadVoltageADS1115()
{
    uint8_t config[3];
    uint8_t data[2];
    int16_t raw;

    // Register configuration (0x01)
    config[0] = 0x01;
    config[1] = 0xC3; // 1100 0011 : OS=1 (start single), MUX=000 (AIN0), PGA=2.048V, MODE=single-shot
    config[2] = 0x83; // 1000 0011 : 128SPS, disable comparator

    HAL_I2C_Master_Transmit(&hi2c2, ADS1115_ADDR, config, 3, HAL_MAX_DELAY);

    HAL_Delay(10); // Wait for conversion (~8ms is enough at 128SPS)

    // Pointer to conversion register
    uint8_t pointer = 0x00;
    HAL_I2C_Master_Transmit(&hi2c2, ADS1115_ADDR, &pointer, 1, HAL_MAX_DELAY);

    // Read the 2 bytes
    HAL_I2C_Master_Receive(&hi2c2, ADS1115_ADDR, data, 2, HAL_MAX_DELAY);

    raw = (data[0] << 8) | data[1];

    // Convert raw value to voltage (if PGA = ±2.048V → LSB = 62.5µV)
    float voltage = (float)raw * 0.000125f; // était 0.0000625f

    return voltage;
}

// Computes temperature from NTC voltage using Steinhart-Hart approximation : (float) -> float
float ComputeTemperature(float v_ntc)
{
    // Calculation of the thermistor resistance
    float r_ntc = R_FIXED * (V_REF - v_ntc) / v_ntc; // numérateur et dénominateur permutés

    // Simplified Steinhart-Hart formula
    float temp_K = 1.0f / (1.0f / T0 + (1.0f / BETA) * logf(r_ntc / R0));

    return temp_K - 273.15f; // Convert to °C
}
