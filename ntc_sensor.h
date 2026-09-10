/**
 * @file    ntc_sensor.h
 * @brief   Generic NTC Thermistor Temperature Calculation Library.
 * @note    Features:
 *          1. Table-driven architecture decoupled from any specific NTC model or MCU.
 *          2. Fast O(log N) binary search + linear interpolation for fractional degrees.
 *          3. Supports customizable step sizes (e.g. 1 deg C, 2 deg C, 5 deg C) and arbitrary temp spans.
 *          4. Provides float (deg C) and fixed-point (0.1 deg C integer) calculation APIs.
 *          5. Built-in voltage divider network resistance calculation helpers.
 * @version V1.0
 * @date    2026-09-10
 * @par Change Log:
 * Date       Version  Author                       Description
 * 2026-09-10  V1.0    Rougga(2839754468@qq.com)    Initial release
 * 
 */

#ifndef NTC_SENSOR_H
#define NTC_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic NTC Thermistor Calibration Table Descriptor
 */
typedef struct {
    const char     *name;           /**< Model identifier string (e.g. "MF52A103F3950") */
    int16_t         min_temp_c;     /**< Minimum table temperature in deg C (e.g. -55) */
    int16_t         max_temp_c;     /**< Maximum table temperature in deg C (e.g. 125) */
    uint8_t         temp_step_c;    /**< Temperature step between consecutive table points (e.g. 1) */
    uint16_t        table_size;     /**< Total number of points in table_ohm */
    const uint32_t *table_ohm;      /**< Monotonically decreasing resistance array in Ohms */
} ntc_table_t;

/* ========================================================================== */
/* Core Conversion APIs                                                       */
/* ========================================================================== */

/**
 * @brief Convert measured NTC resistance in Ohms to temperature in degrees Celsius (float).
 * @param ntc Pointer to the generic NTC table descriptor.
 * @param res_ohm Measured resistance across NTC in Ohms.
 * @param temp_c Pointer to store resulting temperature in deg C (e.g. 25.18).
 * @return true if resistance is within table temperature range,
 *         false if open-circuit, short-circuit, or invalid argument.
 */
bool ntc_ohm_to_temp_c(const ntc_table_t *ntc, uint32_t res_ohm, float *temp_c);

/**
 * @brief Convert measured NTC resistance in Ohms to deci-degrees (0.1 deg C integer).
 * @note  Suitable for resource-constrained systems without FPU formatting (e.g. 251 represents 25.1 deg C).
 * @param ntc Pointer to the generic NTC table descriptor.
 * @param res_ohm Measured resistance in Ohms.
 * @param temp_0_1c Pointer to store resulting temperature in 0.1 deg C.
 * @return true if valid, false if out of range.
 */
bool ntc_ohm_to_temp_0_1c(const ntc_table_t *ntc, uint32_t res_ohm, int16_t *temp_0_1c);

/**
 * @brief Get nominal resistance in Ohms for a given integer temperature.
 * @param ntc Pointer to the generic NTC table descriptor.
 * @param temp_c Target temperature in deg C.
 * @param res_ohm Pointer to store nominal resistance in Ohms.
 * @return true if temperature is valid and within table range, false otherwise.
 */
bool ntc_temp_c_to_ohm(const ntc_table_t *ntc, int16_t temp_c, uint32_t *res_ohm);

/* ========================================================================== */
/* Temperature Unit Conversion Helpers (Celsius <-> Kelvin)                  */
/* ========================================================================== */

#define NTC_CELSIUS_TO_KELVIN_OFFSET_FLOAT    (273.15f)
#define NTC_CELSIUS_TO_KELVIN_OFFSET_0_1K     (2731)     /**< 273.15 * 10 rounded to 2731 for 0.1K format */

/**
 * @brief Convert Celsius (deg C) to Kelvin (K) in float.
 * @param celsius Temperature in deg C.
 * @return Temperature in Kelvin.
 */
static inline float ntc_celsius_to_kelvin_f(float celsius)
{
    return celsius + NTC_CELSIUS_TO_KELVIN_OFFSET_FLOAT;
}

/**
 * @brief Convert Kelvin (K) to Celsius (deg C) in float.
 * @param kelvin Temperature in Kelvin.
 * @return Temperature in deg C.
 */
static inline float ntc_kelvin_to_celsius_f(float kelvin)
{
    return kelvin - NTC_CELSIUS_TO_KELVIN_OFFSET_FLOAT;
}

/**
 * @brief Convert deci-Celsius (0.1 deg C) to deci-Kelvin (0.1 K) integer.
 * @note  Example: 250 (25.0 C) -> 2981 (298.1 K)
 * @param temp_0_1c Temperature in 0.1 deg C.
 * @return Temperature in 0.1 K.
 */
static inline int32_t ntc_celsius_to_kelvin_0_1k(int16_t temp_0_1c)
{
    return (int32_t)temp_0_1c + (int32_t)NTC_CELSIUS_TO_KELVIN_OFFSET_0_1K;
}

/**
 * @brief Convert deci-Kelvin (0.1 K) to deci-Celsius (0.1 deg C) integer.
 * @note  Example: 2981 (298.1 K) -> 250 (25.0 C)
 * @param temp_0_1k Temperature in 0.1 K.
 * @return Temperature in 0.1 deg C.
 */
static inline int16_t ntc_kelvin_to_celsius_0_1c(int32_t temp_0_1k)
{
    return (int16_t)(temp_0_1k - (int32_t)NTC_CELSIUS_TO_KELVIN_OFFSET_0_1K);
}

/**
 * @brief Convert measured NTC resistance in Ohms directly to Kelvin (K, float).
 * @param ntc Pointer to the generic NTC table descriptor.
 * @param res_ohm Measured resistance in Ohms.
 * @param temp_k Pointer to store resulting temperature in Kelvin.
 * @return true on success within valid table envelope, false otherwise.
 */
bool ntc_ohm_to_temp_k(const ntc_table_t *ntc, uint32_t res_ohm, float *temp_k);

/**
 * @brief Convert measured NTC resistance in Ohms directly to deci-Kelvin (0.1 K integer).
 * @param ntc Pointer to the generic NTC table descriptor.
 * @param res_ohm Measured resistance in Ohms.
 * @param temp_0_1k Pointer to store resulting temperature in 0.1 K.
 * @return true on success within valid table envelope, false otherwise.
 */
bool ntc_ohm_to_temp_0_1k(const ntc_table_t *ntc, uint32_t res_ohm, int32_t *temp_0_1k);

/* ========================================================================== */
/* Voltage Divider Hardware Calculation Helpers                               */
/* ========================================================================== */

/**
 * @brief Calculate NTC resistance from ADC counts in a Pull-Up divider configuration.
 * @note  Circuit: VCC -> Pull-up Resistor (R_pullup) -> ADC node -> NTC -> GND
 *        Formula: R_ntc = (adc_raw * R_pullup) / (adc_max - adc_raw)
 * @param adc_raw Raw ADC conversion count at the NTC node.
 * @param adc_max Full-scale maximum count of ADC (e.g. 4095 for 12-bit ADC).
 * @param pullup_ohm Pull-up reference resistor value in Ohms (e.g. 10000 for 10k).
 * @param out_res_ohm Pointer to store calculated NTC resistance in Ohms.
 * @return true on success, false on ADC saturation (adc_raw >= adc_max) or divide-by-zero.
 */
bool ntc_calc_res_pullup_divider(uint32_t adc_raw, uint32_t adc_max, uint32_t pullup_ohm, uint32_t *out_res_ohm);

/**
 * @brief Calculate NTC resistance from ADC counts in a Pull-Down divider configuration.
 * @note  Circuit: VCC -> NTC -> ADC node -> Pull-down Resistor (R_pulldown) -> GND
 *        Formula: R_ntc = ((adc_max - adc_raw) * R_pulldown) / adc_raw
 * @param adc_raw Raw ADC conversion count at the NTC node.
 * @param adc_max Full-scale maximum count of ADC (e.g. 4095 for 12-bit ADC).
 * @param pulldown_ohm Pull-down reference resistor value in Ohms.
 * @param out_res_ohm Pointer to store calculated NTC resistance in Ohms.
 * @return true on success, false if adc_raw == 0 or adc_raw > adc_max.
 */
bool ntc_calc_res_pulldown_divider(uint32_t adc_raw, uint32_t adc_max, uint32_t pulldown_ohm, uint32_t *out_res_ohm);

#ifdef __cplusplus
}
#endif

#endif /* NTC_SENSOR_H */
