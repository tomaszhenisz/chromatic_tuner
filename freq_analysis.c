#include "freq_analysis.h"

uint16_t min_in_range(int32_t array[], uint16_t begin_index, uint16_t range)
{
    uint16_t min_index = begin_index;
    for (uint16_t i = 0; i < range; i++)
    {
        if (array[begin_index + i] < array[min_index])
            min_index = begin_index + i;
    }
    return min_index;
}

void calculate_peaks(uint16_t peaks[], uint8_t *peak_count, int32_t array[])
{
    uint16_t prev_min_index = min_in_range(array, 0, PEAK_SEARCH_RANGE);
    uint16_t current_min_index = min_in_range(array, PEAK_SEARCH_RANGE, PEAK_SEARCH_RANGE);
    uint16_t next_min_index;

    for (uint16_t i = PEAK_SEARCH_RANGE; i < SHIFT_LIMIT - PEAK_SEARCH_RANGE - PEAK_SEARCH_RANGE; i += PEAK_SEARCH_RANGE)
    {
        next_min_index = min_in_range(array, i + PEAK_SEARCH_RANGE, PEAK_SEARCH_RANGE);

        if (array[prev_min_index] > array[current_min_index] && array[next_min_index] > array[current_min_index])
        {
            // Eliminate local minimums that have a value greater than the previous one.
            // This step is crucial in filtering odd harmonics, especially the 3rd.
            if (current_min_index > 0 && array[current_min_index - 1] < array[current_min_index])
                break;

            (*peak_count)++;
            printf("FOUND LOCAL MINIMUM! index: %d, val: %d, peak_count: %d\n", current_min_index, array[current_min_index], *peak_count);

            // Add peak index to array
            peaks[(*peak_count) - 1] = current_min_index;
            if (*peak_count >= PEAK_TRACKING_LIMIT)
                return;
        }
        prev_min_index = current_min_index;
        current_min_index = next_min_index;
    }
}

float calculate_avg_wavelength(uint16_t peaks[], uint8_t peak_count)
{
    if (peak_count == 0)
        return DEFAULT_VAL;

    float sum = 0;
    for (uint8_t i = 0; i < peak_count; i++)
    {
        sum += (float)peaks[i] / (float)(i + 1);
    }
    return sum / peak_count;
}

uint16_t calculate_sma(uint16_t index, uint8_t array[])
{
    uint16_t sum = 0;
    for (uint8_t i = 0; i <= SMA_WIDTH; i++)
    {
        sum += array[index + i];
    }
    return sum / (SMA_WIDTH + 1);
}

int32_t calculate_interference_pwr(int shift, uint8_t array[])
{
    int32_t power_diff = 0;
    for (uint16_t i = 0; i < NUM_SAMPLES - shift; i++)
    {
        power_diff += abs(array[i] - array[i + shift]);
        // The line below is inserted to save some calculation. If there is a need to plot and observe interference function, it can be commented out.
        if (power_diff > INTERFERENCE_THRESHOLD)
            return INT_MAX;
    }
    return power_diff;
}

float calculate_freq(uint8_t array[])
{
    int32_t interference[NUM_SAMPLES];
    for (uint16_t shift = 0; shift < NUM_SAMPLES; shift++)
    {
        interference[shift] = calculate_interference_pwr(shift, array);
    }

    uint8_t peak_count = 0;
    uint16_t peaks[PEAK_TRACKING_LIMIT];
    calculate_peaks(peaks, &peak_count, interference);

    float avg_wavelength = calculate_avg_wavelength(peaks, peak_count);
    float frequency = FS / avg_wavelength;
    return frequency;
}