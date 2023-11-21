#ifndef MACROS_H
#define MACROS_H

#define NUM_SAMPLES 1500            // The number of gathered samples.
#define SHIFT_LIMIT 1250            // Max phase shift to investigate. Must be lower than NUM_SAMPLES.
                                    // Emperically tested that values close to NUM_SAMPLE may cause fainding a wrong peak due to noise, and disrupt the results.
#define SMA_WIDTH 20                // Number of samples to average out when calculating Simple Movin Average.
#define PEAK_SEARCH_RANGE 15        // The width of peak search. It's assumed that peaks should not be separated by less than 2*PEAK_SEARCH_RANGE samples.
#define PEAK_TRACKING_LIMIT 10      // Maximum peak count to track
#define INTERFERENCE_THRESHOLD 3000 // Peaks of value higher that INTERFERENCE_THRESHOLD are ignored, thus if interference exceeds this value, its calculation can be aborted
#define TUNE_PRECISION 0.7          // Tuning precision
#define DEFAULT_VAL 100

#define SEGMENT_A_PIN  9            // Segment A wired to GP9
#define SEGMENT_B_PIN  8
#define SEGMENT_C_PIN  7
#define SEGMENT_D_PIN  10
#define SEGMENT_E_PIN  11
#define SEGMENT_F_PIN  12
#define SEGMENT_G_PIN  13
#define SEGMENT_DP_PIN  6

#define LOW_PITCH_INDICATOR_PIN 18  // Low pitch indicator led wired to GP18
#define IN_TUNE_INDICATOR_PIN 19    // In tune indicator led wired to GP19
#define HI_PITCH_INDICATOR_PIN 20   // Hi pitch indicator led wired to GP20

//Segments to display a note. Each bit represent a segment of a display.
//The display used is common anode, so 0 implies a lit segment.
#define A_note 0b00010001
#define A_sharp_note 0b00010000
#define B_note 0b11000001
#define C_note 0b01100011
#define C_sharp_note 0b01100010
#define D_note 0b10000101
#define D_sharp_note 0b10000100
#define E_note 0b01100001
#define F_note 0b01110001
#define F_sharp_note 0b01110000
#define G_note 0b01000011
#define G_sharp_note 0b01000010

// Ranges for tuning given note
#define A3_bottom_range 213.74       // 220.00Hz / 1.0293 (quarter tone ratio)
#define A3_sharp_bottom_range 226.45 // 233.08Hz / 1.0293
#define B3_bottom_range 239.91       // 246.94Hz / 1.0293
#define C3_bottom_range 254.18       // 261.63Hz / 1.0293
#define C3_sharp_bottom_range 269.29 // 277.18Hz / 1.0293
#define D3_bottom_range 285.30       // 293.66Hz / 1.0293
#define D3_sharp_bottom_range 302.27 // 311.13Hz / 1.0293
#define E3_bottom_range 320.25       // 329.63Hz / 1.0293
#define F3_bottom_range 339.29       // 349.23Hz / 1.0293
#define F3_sharp_bottom_range 359.46 // 369.99Hz / 1.0293
#define G3_bottom_range 380.849      // 392.00Hz / 1.0293
#define G3_sharp_bottom_range 403.48 // 415.30Hz / 1.0293
#define A4_bottom_range 427.47       // 440.00Hz / 1.0293

// Frequencies of musical notes
#define A3_freq 220.00 // Hz
#define A3_sharp_freq 233.08 // Hz
#define B3_freq 246.94 // Hz
#define C3_freq 261.63 // Hz
#define C3_sharp_freq 277.18 // Hz
#define D3_freq 293.66 // Hz
#define D3_sharp_freq 311.13 // Hz
#define E3_freq 329.63 // Hz
#define F3_freq 349.23 // Hz
#define F3_sharp_freq 369.99 // Hz
#define G3_freq 392.00 // Hz
#define G3_sharp_freq 415.30 // Hz
#define A4_freq 440.00 // Hz

#define ADC_CHAN 0          // ADC mux value (0 for input 26)
#define ADC_PIN 26          // ADC input pin
#define FS 44000            // Sampling freq. Increasing is unlikely to improve tuner operation. 44000 is probably still an overkill.
#define ADCCLK 48000000.0   // Internal ADC clock freq, not adjustable

#endif