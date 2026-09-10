/**
 * @file    ntc_sensor.c
 * @brief   Generic NTC Thermistor Conversion Core Implementation.
 * @version V1.0
 * @date    2026-09-10
 * @par Change Log:
 * Date       Version  Author                       Description
 * 2026-09-10  V1.0    Rougga(2839754468@qq.com)    Initial release
 * 
 */

#include "ntc_sensor.h"

bool ntc_ohm_to_temp_c(const ntc_table_t *ntc, uint32_t res_ohm, float *temp_c)
{
    if (ntc == NULL || ntc->table_ohm == NULL || ntc->table_size < 2 || temp_c == NULL) {
        return false;
    }

    uint32_t r_max = ntc->table_ohm[0];                      /* Resistance at minimum temperature */
    uint32_t r_min = ntc->table_ohm[ntc->table_size - 1];     /* Resistance at maximum temperature */

    /* Out of bounds: open circuit or short circuit */
    if (res_ohm > r_max || res_ohm < r_min) {
        return false;
    }

    if (res_ohm == r_max) {
        *temp_c = (float)ntc->min_temp_c;
        return true;
    }
    if (res_ohm == r_min) {
        *temp_c = (float)ntc->max_temp_c;
        return true;
    }

    /* Binary search for interval [idx, idx + 1] where table[idx] >= res_ohm > table[idx + 1] */
    int low = 0;
    int high = (int)ntc->table_size - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (ntc->table_ohm[mid] == res_ohm) {
            uint8_t step = (ntc->temp_step_c > 0) ? ntc->temp_step_c : 1U;
            *temp_c = (float)(ntc->min_temp_c + (int16_t)(mid * step));
            return true;
        } else if (ntc->table_ohm[mid] > res_ohm) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    int idx = high;
    uint32_t r_high = ntc->table_ohm[idx];
    uint32_t r_low  = ntc->table_ohm[idx + 1];

    if (r_high <= r_low) {
        return false; /* Invalid monotonic table */
    }

    uint8_t step = (ntc->temp_step_c > 0) ? ntc->temp_step_c : 1U;
    float base_temp = (float)(ntc->min_temp_c + (int16_t)(idx * step));
    float fraction  = (float)(r_high - res_ohm) / (float)(r_high - r_low);

    *temp_c = base_temp + fraction * (float)step;
    return true;
}

bool ntc_ohm_to_temp_0_1c(const ntc_table_t *ntc, uint32_t res_ohm, int16_t *temp_0_1c)
{
    if (ntc == NULL || ntc->table_ohm == NULL || ntc->table_size < 2 || temp_0_1c == NULL) {
        return false;
    }

    uint32_t r_max = ntc->table_ohm[0];
    uint32_t r_min = ntc->table_ohm[ntc->table_size - 1];

    if (res_ohm > r_max || res_ohm < r_min) {
        return false;
    }

    uint8_t step = (ntc->temp_step_c > 0) ? ntc->temp_step_c : 1U;

    if (res_ohm == r_max) {
        *temp_0_1c = (int16_t)(ntc->min_temp_c * 10);
        return true;
    }
    if (res_ohm == r_min) {
        *temp_0_1c = (int16_t)(ntc->max_temp_c * 10);
        return true;
    }

    int low = 0;
    int high = (int)ntc->table_size - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (ntc->table_ohm[mid] == res_ohm) {
            *temp_0_1c = (int16_t)((ntc->min_temp_c + (int16_t)(mid * step)) * 10);
            return true;
        } else if (ntc->table_ohm[mid] > res_ohm) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    int idx = high;
    uint32_t r_high = ntc->table_ohm[idx];
    uint32_t r_low  = ntc->table_ohm[idx + 1];

    if (r_high <= r_low) {
        return false;
    }

    int32_t base_0_1c = (int32_t)(ntc->min_temp_c + (int16_t)(idx * step)) * 10;
    int32_t step_10   = (int32_t)step * 10;
    int32_t frac_0_1c = (int32_t)(((uint64_t)(r_high - res_ohm) * (uint64_t)step_10 + ((r_high - r_low) / 2ULL)) / (r_high - r_low));

    *temp_0_1c = (int16_t)(base_0_1c + frac_0_1c);
    return true;
}

bool ntc_temp_c_to_ohm(const ntc_table_t *ntc, int16_t temp_c, uint32_t *res_ohm)
{
    if (ntc == NULL || ntc->table_ohm == NULL || res_ohm == NULL ||
        temp_c < ntc->min_temp_c || temp_c > ntc->max_temp_c) {
        return false;
    }

    uint8_t step = (ntc->temp_step_c > 0) ? ntc->temp_step_c : 1U;
    int idx = (temp_c - ntc->min_temp_c) / step;

    if (idx < 0 || idx >= (int)ntc->table_size) {
        return false;
    }

    *res_ohm = ntc->table_ohm[idx];
    return true;
}

bool ntc_ohm_to_temp_k(const ntc_table_t *ntc, uint32_t res_ohm, float *temp_k)
{
    if (temp_k == NULL) {
        return false;
    }

    float temp_c = 0.0f;
    if (!ntc_ohm_to_temp_c(ntc, res_ohm, &temp_c)) {
        return false;
    }

    *temp_k = ntc_celsius_to_kelvin_f(temp_c);
    return true;
}

bool ntc_ohm_to_temp_0_1k(const ntc_table_t *ntc, uint32_t res_ohm, int32_t *temp_0_1k)
{
    if (temp_0_1k == NULL) {
        return false;
    }

    int16_t temp_0_1c = 0;
    if (!ntc_ohm_to_temp_0_1c(ntc, res_ohm, &temp_0_1c)) {
        return false;
    }

    *temp_0_1k = ntc_celsius_to_kelvin_0_1k(temp_0_1c);
    return true;
}

bool ntc_calc_res_pullup_divider(uint32_t adc_raw, uint32_t adc_max, uint32_t pullup_ohm, uint32_t *out_res_ohm)
{
    if (out_res_ohm == NULL || adc_raw >= adc_max || adc_max == 0 || pullup_ohm == 0) {
        return false;
    }

    *out_res_ohm = (uint32_t)(((uint64_t)adc_raw * (uint64_t)pullup_ohm) / (uint64_t)(adc_max - adc_raw));
    return true;
}

bool ntc_calc_res_pulldown_divider(uint32_t adc_raw, uint32_t adc_max, uint32_t pulldown_ohm, uint32_t *out_res_ohm)
{
    if (out_res_ohm == NULL || adc_raw == 0 || adc_raw > adc_max || pulldown_ohm == 0) {
        return false;
    }

    *out_res_ohm = (uint32_t)(((uint64_t)(adc_max - adc_raw) * (uint64_t)pulldown_ohm) / (uint64_t)adc_raw);
    return true;
}
