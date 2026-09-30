/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_ina219_basic.c
 * @brief     driver ina219 basic source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2021-06-13
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/06/13  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ina219_basic.h"

static ina219_handle_t gs_handle[INA219_SENSOR_COUNT]; /**< ina219 handles, one per sensor */

/**
 * @brief     basic example init
 * @param[in] id sensor identifier
 * @param[in] addr_pin iic address pin
 * @param[in] r reference resistor value
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t ina219_basic_init(ina219_sensor_id_t id, ina219_address_t addr_pin, double r)
{
    uint8_t res;
    uint16_t calibration;
    ina219_handle_t *handle;

    if (id >= INA219_SENSOR_COUNT)
    {
        return 1;
    }
    handle = &gs_handle[id];

    /* link interface function */
    DRIVER_INA219_LINK_INIT(handle, ina219_handle_t);
    DRIVER_INA219_LINK_IIC_INIT(handle, ina219_interface_iic_init);
    DRIVER_INA219_LINK_IIC_DEINIT(handle, ina219_interface_iic_deinit);
    DRIVER_INA219_LINK_IIC_READ(handle, ina219_interface_iic_read);
    DRIVER_INA219_LINK_IIC_WRITE(handle, ina219_interface_iic_write);
    DRIVER_INA219_LINK_DELAY_MS(handle, ina219_interface_delay_ms);
    DRIVER_INA219_LINK_DEBUG_PRINT(handle, ina219_interface_debug_print);

    /* set addr pin */
    res = ina219_set_addr_pin(handle, addr_pin);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set addr pin failed.\n");

        return 1;
    }

    /* set the r */
    res = ina219_set_resistance(handle, r);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set resistance failed.\n");

        return 1;
    }

    /* init */
    res = ina219_init(handle);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: init failed.\n");

        return 1;
    }

    /* set bus voltage range */
    res = ina219_set_bus_voltage_range(handle, INA219_BASIC_DEFAULT_BUS_VOLTAGE_RANGE);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set bus voltage range failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    /* set bus voltage adc mode */
    res = ina219_set_bus_voltage_adc_mode(handle, INA219_BASIC_DEFAULT_BUS_VOLTAGE_ADC_MODE);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set bus voltage adc mode failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    /* set shunt voltage adc mode */
    res = ina219_set_shunt_voltage_adc_mode(handle, INA219_BASIC_DEFAULT_SHUNT_VOLTAGE_ADC_MODE);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set shunt voltage adc mode failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    /* set shunt bus voltage continuous */
    res = ina219_set_mode(handle, INA219_MODE_SHUNT_BUS_VOLTAGE_CONTINUOUS);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set mode failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    /* set pga */
    res = ina219_set_pga(handle, INA219_BASIC_DEFAULT_PGA);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set pga failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    /* calculate calibration */
    res = ina219_calculate_calibration(handle, (uint16_t *)&calibration);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: calculate calibration failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    /* set calibration */
    res = ina219_set_calibration(handle, calibration);
    if (res != 0)
    {
        ina219_interface_debug_print("ina219: set calibration failed.\n");
        (void)ina219_deinit(handle);

        return 1;
    }

    return 0;
}

/**
 * @brief      basic example read
 * @param[in]  id sensor identifier
 * @param[out] *mV pointer to a mV buffer
 * @param[out] *mA pointer to a mA buffer
 * @param[out] *mW pointer to a mW buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t ina219_basic_read(ina219_sensor_id_t id, float *mV, float *mA, float *mW)
{
    uint8_t res;
    int16_t s_raw;
    uint16_t u_raw;
    ina219_handle_t *handle;

    if (id >= INA219_SENSOR_COUNT)
    {
        return 1;
    }
    handle = &gs_handle[id];

    /* read bus voltage */
    res = ina219_read_bus_voltage(handle, (uint16_t *)&u_raw, mV);
    if (res != 0)
    {
        return 1;
    }

    /* read current */
    res = ina219_read_current(handle, (int16_t *)&s_raw, mA);
    if (res != 0)
    {
        return 1;
    }

    /* read power */
    res = ina219_read_power(handle, (uint16_t *)&u_raw, mW);
    if (res != 0)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief     basic example deinit
 * @param[in] id sensor identifier
 * @return    status code
 *            - 0 success
 *            - 1 deinit failed
 * @note      none
 */
uint8_t ina219_basic_deinit(ina219_sensor_id_t id)
{
    uint8_t res;

    if (id >= INA219_SENSOR_COUNT)
    {
        return 1;
    }

    res = ina219_deinit(&gs_handle[id]);
    if (res != 0)
    {
        return 1;
    }

    return 0;
}
