/**
 * Tomasz Henisz (thenisz@gmail.com)
 * 
 * This demonstration is a chromatic tuner implemented on Raspberry Pi Pico board.
 * The input is provided by an electret microphone with a preamp (no automatic gain control).
 * Tuning information is presented in real time, on a 7-segment display (with a dot representing # symbol) and 3 LEDs.
 * Note, that if used with a different mic/preamp, signal level and SNR may vary, so fine tuning the parameters in <macros.h> might be required.
 * 
 * About the method:
 * The method does not involve FFT (Fast Fourier Transform), as many of available projects.
 * The reason for this choice, not getting too deep into maths, is that in case of FFT, obtaining a satysfying tuning precision
 * requires lowering the refresh rate below acceptable, and vice-versa.
 * The method is based on an assumption that if there is some periodic input signal (let the period last for 100 samples for easy calculation),
 * then if this signal is shifted by a multitude of its period (100, 200, 300 ...) samples and subtracted from the original signal, they should cancel out.
 * So, it should be possible to find the shift values that provide the best cancellation, and use them to estimate the base frequency of the input.
 * 
 * List of components
 * - LEDs - 3pcs (typically 2 red and 1 green)
 * - 7-segment, common anode display with a dot
 * - microphone with a preamp
 * - 330Ohm voltage limiting resistors - 11pcs
 * 
 * Wiring:
 * PIN12 (GP9) --------> |330Ohm| -------> Dissplay Segment A
 * PIN11 (GP8) --------> |330Ohm| -------> Dissplay Segment B
 * PIN10 (GP7) --------> |330Ohm| -------> Dissplay Segment C
 * PIN14 (GP10) -------> |330Ohm| -------> Dissplay Segment D
 * PIN15 (GP11) -------> |330Ohm| -------> Dissplay Segment E
 * PIN16 (GP12) -------> |330Ohm| -------> Dissplay Segment F
 * PIN17 (GP13) -------> |330Ohm| -------> Dissplay Segment G
 * PIN9  (GP6) --------> |330Ohm| -------> Dissplay Segment DP
 * PIN36 (3V3) --------------------------> Dissplay's common anode
 * 
 * PIN36 (3V3) --------------------------> Microphone VCC
 * PIN38 (GND) --------------------------> Microphone GND
 * PIN31 (GP26) -------------------------> Microphone OUT
 * 
 * PIN24 (GP18) -------> |330Ohm| -------> LED Anode ----------> PIN23(GND)
 * PIN25 (GP19) -------> |330Ohm| -------> LED Anode ----------> PIN23(GND)
 * PIN26 (GP120) ------> |330Ohm| -------> LED Anode ----------> PIN23(GND)
 * 
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/dma.h"
#include "hardware/adc.h"
#include "hardware/irq.h"

#include "macros.h"
#include "freq_analysis.h"

// DMA channels for ADC
uint8_t sample_channel = 0;
uint8_t control_channel = 1; // resetting write_addr of sample_channel

// Destination for DMA to transfer samples from ADC
// The size is incremented by SMA_WIDTH to provide extra samples for Simple Moving Average (SMA) smoothing
uint8_t samples_buff[NUM_SAMPLES + SMA_WIDTH];

// Pointer to the sample buffer
uint8_t *samples_buff_ptr = &samples_buff[0];

// Union to push float through FIFO
union frequency_union
{
    float f;
    uint32_t i;
};

/**
 * @brief Core 0 Thread Function
 *
 * This function runs on Core 0 and continuously performs the following tasks:
 *
 * 1. Waits for samples from an ADC using DMA.
 * 2. Copies the samples from the buffer, applying Simple Moving Average (SMA) smoothing.
 * 3. Restarts the sample DMA channel, allowing collection of the next sample set.
 * 4. Calculates the base frequency of the input signal using the smoothed samples.
 * 5. Passes the calculated frequency to Core 1 using the multicore FIFO.
 *
 * The function operates in an infinite loop, ensuring continuous processing of incoming samples.
 */
void core0_thread()
{
    float frequency;
    uint8_t samples[NUM_SAMPLES];

    while (1)
    {
        // Wait for samples from ADC
        dma_channel_wait_for_finish_blocking(sample_channel);

        // Copy samples from sampes_buff, applying SMA smoothing
        for (uint16_t i = 0; i < NUM_SAMPLES; i++)
        {
            samples[i] = calculate_sma(i, samples_buff);
        }

        // Restart the sample channel, samples_buff can be overwritten
        dma_channel_start(control_channel);

        // Calculate the base freq of the input signal
        frequency = calculate_freq(samples);

        // Pass calculated freq to core_1 and start over
        union frequency_union frequency_union;
        frequency_union.f = frequency;
        multicore_fifo_push_blocking(frequency_union.i);
    }
}

/**
 * @brief Update Display Function
 *
 * This function updates a 7-segment LED display based on the provided note value.
 * The note value is a binary representation where each bit corresponds to a specific
 * segment of the 7-segment display.
 *
 * @param note The binary representation of the segments to be presented on the 7-segment display.
 */
void update_display(uint8_t note)
{
    gpio_put(SEGMENT_A_PIN, (note & 0b10000000));
    gpio_put(SEGMENT_B_PIN, (note & 0b01000000));
    gpio_put(SEGMENT_C_PIN, (note & 0b00100000));
    gpio_put(SEGMENT_D_PIN, (note & 0b00010000));
    gpio_put(SEGMENT_E_PIN, (note & 0b00001000));
    gpio_put(SEGMENT_F_PIN, (note & 0b00000100));
    gpio_put(SEGMENT_G_PIN, (note & 0b00000010));
    gpio_put(SEGMENT_DP_PIN, (note & 0b00000001));
}

/**
 * @brief Update LEDs Function
 *
 * This function updates indicator LEDs based on the difference between
 * a provided frequency and a reference frequency. The 3 LEDs represent different pitch
 * ranges, indicating whether the input frequency is below, within, or above the
 * acceptable tuning precision.
 *
 * @param frequency The input frequency to be compared.
 * @param reference_frequency The reference frequency for comparison.
 */
void update_leds(float frequency, float refference_frequency)
{
    if (frequency - refference_frequency < -TUNE_PRECISION)
    {
        gpio_put(LOW_PITCH_INDICATOR_PIN, 1);
        gpio_put(IN_TUNE_INDICATOR_PIN, 0);
        gpio_put(HI_PITCH_INDICATOR_PIN, 0);
    }
    else if (frequency - refference_frequency > TUNE_PRECISION)
    {
        gpio_put(LOW_PITCH_INDICATOR_PIN, 0);
        gpio_put(IN_TUNE_INDICATOR_PIN, 0);
        gpio_put(HI_PITCH_INDICATOR_PIN, 1);
    }
    else
    {
        gpio_put(LOW_PITCH_INDICATOR_PIN, 0);
        gpio_put(IN_TUNE_INDICATOR_PIN, 1);
        gpio_put(HI_PITCH_INDICATOR_PIN, 0);
    }
}

/**
 * @brief Core 1 Interrupt Handler Function
 *
 * This function serves as the interrupt handler for Core 1.
 * It is responsible for processing incoming frequency valueta from Core 0, updating the display, and controlling LEDs.
 */
void core1_interrupt_handler()
{
    while (multicore_fifo_rvalid())
    {
        union frequency_union frequency_union;
        frequency_union.i = multicore_fifo_pop_blocking();
        float frequency = frequency_union.f;

        //Print receiver freq to console
        printf("\nCore_1: %fHz\n", frequency);

        // Normalize received frequency to fit described range.
        // Division/Multiplication by 2 changes the octave, so for example C4 note will be changed to C3
        while (frequency < A3_bottom_range)
            frequency *= 2;
        while (frequency > A4_bottom_range)
            frequency /= 2;

        // Fit the frequewncy to the proper range and update output
        if (frequency > A3_bottom_range && frequency < A3_sharp_bottom_range)
        {
            update_display(A_note);
            update_leds(frequency, A3_freq);
        }
        else if (frequency > A3_sharp_bottom_range && frequency < B3_bottom_range)
        {
            update_display(A_sharp_note);
            update_leds(frequency, A3_sharp_freq);
        }
        else if (frequency > B3_bottom_range && frequency < C3_bottom_range)
        {
            update_display(B_note);
            update_leds(frequency, B3_freq);
        }
        else if (frequency > C3_bottom_range && frequency < C3_sharp_bottom_range)
        {
            update_display(C_note);
            update_leds(frequency, C3_freq);
        }
        else if (frequency > C3_sharp_bottom_range && frequency < D3_bottom_range)
        {
            update_display(C_sharp_note);
            update_leds(frequency, C3_sharp_freq);
        }
        else if (frequency > D3_bottom_range && frequency < D3_sharp_bottom_range)
        {
            update_display(D_note);
            update_leds(frequency, D3_freq);
        }
        else if (frequency > D3_sharp_bottom_range && frequency < E3_bottom_range)
        {
            update_display(D_sharp_note);
            update_leds(frequency, D3_sharp_freq);
        }
        else if (frequency > E3_bottom_range && frequency < F3_bottom_range)
        {
            update_display(E_note);
            update_leds(frequency, E3_freq);
        }
        else if (frequency > F3_bottom_range && frequency < F3_sharp_bottom_range)
        {
            update_display(F_note);
            update_leds(frequency, F3_freq);
        }
        else if (frequency > F3_sharp_bottom_range && frequency < G3_bottom_range)
        {
            update_display(F_sharp_note);
            update_leds(frequency, F3_sharp_freq);
        }
        else if (frequency > G3_bottom_range && frequency < G3_sharp_bottom_range)
        {
            update_display(G_note);
            update_leds(frequency, G3_freq);
        }
        else if (frequency > G3_sharp_bottom_range && frequency < A4_bottom_range)
        {
            update_display(G_sharp_note);
            update_leds(frequency, G3_sharp_freq);
        }
    }
    multicore_fifo_clear_irq();
}

/**
 * @brief Core 1 Entry Function
 *
 * This function serves as the entry point for Core 1.
 * It enables SIO interrupt for core 1, and assigns the exclusive interrupt handler, thatwhich is run when data is received from the FIFO.
 */
void core1_entry()
{
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);
    while (1)
    {
        tight_loop_contents();
    }
}

void init_segment_display()
{
    //Initialise GPIOs
    gpio_init(SEGMENT_A_PIN);
    gpio_init(SEGMENT_B_PIN);
    gpio_init(SEGMENT_C_PIN);
    gpio_init(SEGMENT_D_PIN);
    gpio_init(SEGMENT_E_PIN);
    gpio_init(SEGMENT_F_PIN);
    gpio_init(SEGMENT_G_PIN);
    gpio_init(SEGMENT_DP_PIN);

    //Set GPIOs as outputs
    gpio_set_dir(SEGMENT_A_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_B_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_C_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_D_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_E_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_F_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_G_PIN, GPIO_OUT);
    gpio_set_dir(SEGMENT_DP_PIN, GPIO_OUT);
}


void init_leds()
{
    //Initialise GPIOs
    gpio_init(LOW_PITCH_INDICATOR_PIN);
    gpio_init(IN_TUNE_INDICATOR_PIN);
    gpio_init(HI_PITCH_INDICATOR_PIN);

    //Set GPIOs as outputs
    gpio_set_dir(LOW_PITCH_INDICATOR_PIN, GPIO_OUT);
    gpio_set_dir(IN_TUNE_INDICATOR_PIN, GPIO_OUT);
    gpio_set_dir(HI_PITCH_INDICATOR_PIN, GPIO_OUT);
}

void init_adc()
{
    // Init GPIO for analogue use: hi-Z, no pulls, disable digital input buffer.
    adc_gpio_init(ADC_PIN);

    // Initialise the ADC HW
    adc_init();

    // Select analog mux input, 0 for GPIO 26
    adc_select_input(ADC_CHAN);

    // Setup the FIFO
    adc_fifo_setup(
        true,  // Write each completed conversion to the sample FIFO
        true,  // Enable DREQ
        1,     // dreq_thresh - DREQ asserted when at least 1 sample present
        false, // Disable error bit
        true   // Shift each sample to 8 bits since the 4 LSBs are noise
    );

    adc_set_clkdiv(ADCCLK / FS - 1);
    adc_run(true); // Enable free-running sampling mode
}

void init_dma()
{
    // Channel configurations
    dma_channel_config c2 = dma_channel_get_default_config(sample_channel);
    dma_channel_config c3 = dma_channel_get_default_config(control_channel);

    // ADC SAMPLE CHANNEL
    channel_config_set_transfer_data_size(&c2, DMA_SIZE_8);
    channel_config_set_read_increment(&c2, false);  // read from constant address
    channel_config_set_write_increment(&c2, true);  // increment write address

    // Select a transfer request signal in a channel configuration object
    channel_config_set_dreq(&c2, DREQ_ADC);

    // Configure the channel
    dma_channel_configure(
        sample_channel,
        &c2,                     // channel config
        samples_buff,            // dst
        &adc_hw->fifo,           // src
        NUM_SAMPLES + SMA_WIDTH, // transfer count
        false                    // don't start immediately
    );

    // CONTROL CHANNEL
    channel_config_set_transfer_data_size(&c3, DMA_SIZE_32); // 32-bit txfers
    channel_config_set_read_increment(&c3, false);           // read from constant address
    channel_config_set_write_increment(&c3, false);          // write to constant address
    channel_config_set_chain_to(&c3, sample_channel);        // chain to sample channel

    dma_channel_configure(
        control_channel,
        &c3,                                    // channel config
        &dma_hw->ch[sample_channel].write_addr, // Write address (channel 0 read address)
        &samples_buff_ptr,                      // Read address (POINTER TO AN ADDRESS)
        1,                                      // transfer count
        false                                   // Don't start immediately.
    );

    dma_start_channel_mask((1u << sample_channel));
}

int main()
{
    stdio_init_all();

    init_segment_display();
    init_leds();
    init_adc();
    init_dma();

    // Launch core 1
    multicore_launch_core1(core1_entry);
    core0_thread();
}