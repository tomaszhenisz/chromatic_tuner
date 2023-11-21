#ifndef FREQ_ANALYSIS_H
#define FREQ_ANALYSIS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "macros.h"

/**
 * @brief Finds the index of the minimum value within a specified range of elements.
 *
 * This function searches for the minimum value in the given range of the array
 * and returns the index of that value.
 * Note that this function does not perform boundary checks.
 *
 * @param array The array to search.
 * @param begin_index The starting index of the range.
 * @param range The range of search.
 *
 * @return The index of the minimum value in the specified range.
 */
uint16_t min_in_range(int32_t array[], uint16_t begin_index, uint16_t range);

/**
 * @brief Identifies local minima in the provided array within a predefined search range.
 *
 * This function calculates peaks in the given array that meet specific criteria.
 * It updates the peak count and records the indices of the identified peaks in the
 * provided peaks array. Note that indices greater than peak_count are not modified.
 *
 * @param peaks Pointer to an array to store the indices of identified peaks.
 * @param peak_count Pointer to the variable holding the current count of peaks.
 * @param array The input array in which peaks are to be found.
 */
void calculate_peaks(uint16_t peaks[], uint8_t *peak_count, int32_t array[]);

/**
 * @brief Calculates the average wavelength based on identified peaks.
 *
 * This function computes the average wavelength using the provided array of peak indices.
 * The wavelength is expressed in samples gathered during one period of the input signal,
 * or, in other words, multiplied by the sampling frequency (FS).
 * If no peaks are identified (peak_count is 0), the function returns a default value.
 * The output is calculated as the average of normalized peak indices.
 *
 * @param peaks Pointer to an array containing the indices of identified peaks.
 * @param peak_count The number of identified peaks in the array.
 *
 * @return The calculated average wavelength or a default value if no peaks are found.
 */
float calculate_avg_wavelength(uint16_t peaks[], uint8_t peak_count);

/**
 * @brief Calculates the Simple Moving Average (SMA) at a specified index in the given array.
 *
 * This function computes the SMA by summing up a predefined number of consecutive elements in the array,
 * starting from provided index, and then dividing the sum by the total number of elements.
 *
 * @param index The index in the array for which SMA is calculated.
 * @param array Pointer to an array containing the data for SMA calculation.
 *
 * @return The calculated Simple Moving Average at the specified index.
 */
uint16_t calculate_sma(uint16_t index, uint8_t array[]);

/**
 * @brief Calculates the power of the input signal when interfered with its shifted version.
 *
 * This function computes the amplitude difference between elements of the array and their
 * corresponding shifted elements. It iterates through the array, accumulating the
 * absolute differences. If the accumulated amplitude difference exceeds the predefined
 * threshold, the function returns INT_MAX to prevent excessive computation.
 * Note that to be mathematicaly correct, the return value should be devided by the number
 * of compared elements to represent signal power. In this case it would only add unnecessary division.
 *
 * @param shift The number of positions to shift the array for interference calculation.
 * @param array The input array for interference calculation.
 *
 * @return The calculated powere of interfered signal or INT_MAX if the threshold is exceeded.
 */
int32_t calculate_interference_pwr(int shift, uint8_t array[]);

/**
 * @brief Estimates the base frequency of the input signal using interference analysis.
 *
 * This function analyzes the input array by searching for shift values, that produce destructive interference.
 *
 * @param array The input array containing the signal for frequency analysis.
 *
 * @return The estimated base frequency of the input signal.
 */
float calculate_freq(uint8_t array[]);

#endif