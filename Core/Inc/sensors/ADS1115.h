/*
 * ADS1115.h
 *
 *  Created on: Jun 4, 2025
 *      Author: e.hollier-larousse
 */

#ifndef INC_ADS1115_H_
#define INC_ADS1115_H_

#include "main.h"
#include <math.h>

#define R_FIXED       47000.0f
#define BETA          4090.0f
#define R0            47000.0f
#define T0            298.15f
#define V_REF         3.3f
#define ADS1115_ADDR  (0x48 << 1)

// Lit la tension sur AIN0 du ADS1115
float ReadVoltageADS1115(void);

// Calcule la température (°C) à partir de la tension mesurée
float ComputeTemperature(float v_ntc);

#endif /* INC_ADS1115_H_ */

