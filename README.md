Here is the tuner in action:

https://github.com/tomaszhenisz/chromatic_tunter/assets/151632431/d5a5865e-750c-42c8-b371-922e74a364ac


 
 This demonstration is a chromatic tuner implemented on Raspberry Pi Pico board.
 The input is provided by an electret microphone with a preamp (no automatic gain control).
 Tuning information is presented in real time, on a 7-segment display (with a dot representing # symbol) and 3 LEDs.
 Note, that if used with a different mic/preamp, signal level and SNR may vary, so fine tuning the parameters in <macros.h> might be required.
 
 About the method:
 The method does not involve FFT (Fast Fourier Transform), as many of available projects.
 The reason for this choice, not getting too deep into maths, is that in case of FFT, obtaining a satysfying tuning precision
 requires lowering the refresh rate below acceptable, and vice-versa.
 The method is based on an assumption that if there is some periodic input signal (let the period last for 100 samples for easy calculation),
 then if this signal is shifted by a multitude of its period (100, 200, 300 ...) samples and subtracted from the original signal, they should cancel out.
 So, it should be possible to find the shift values that provide the best cancellation, and use them to estimate the base frequency of the input.
 
 List of components
 - LEDs - 3pcs (typically 2 red and 1 green)
 - 7-segment, common anode display with a dot
 - microphone with a preamp
 - 330Ohm voltage limiting resistors - 11pcs
 
 Wiring:
* PIN12 (GP9) --------> |330Ohm| -------> Dissplay Segment A
* PIN11 (GP8) --------> |330Ohm| -------> Dissplay Segment B
* PIN10 (GP7) --------> |330Ohm| -------> Dissplay Segment C
* PIN14 (GP10) -------> |330Ohm| -------> Dissplay Segment D
* PIN15 (GP11) -------> |330Ohm| -------> Dissplay Segment E
* PIN16 (GP12) -------> |330Ohm| -------> Dissplay Segment F
* PIN17 (GP13) -------> |330Ohm| -------> Dissplay Segment G
* PIN9  (GP6) --------> |330Ohm| -------> Dissplay Segment DP
* PIN36 (3V3) --------------------------> Dissplay's common anode
 
* PIN36 (3V3) --------------------------> Microphone VCC
* PIN38 (GND) --------------------------> Microphone GND
* PIN31 (GP26) -------------------------> Microphone OUT
 
* PIN24 (GP18) -------> |330Ohm| -------> LED Anode ----------> PIN23(GND)
* PIN25 (GP19) -------> |330Ohm| -------> LED Anode ----------> PIN23(GND)
* PIN26 (GP120) ------> |330Ohm| -------> LED Anode ----------> PIN23(GND)
 
