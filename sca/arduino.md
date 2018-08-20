## Leakage
Also embedded devices are vulnerable to EM attacks.
A small loop antenna can be used to pick up such signals.

<img class="imgCenter" src="images/setup-arduino.jpg">

A DES is implemented on the Arduino to search for side-channel effects.
By averaging the spectrogram over multiple traces, the following spectrogram can be obtained (captured at ~3.5MHz).
A repeating pattern occures that is caused by the individual rounds of the DES.

<img class="imgCenter" src="images/arduino-des-mean.jpg">

Even though it was not possible to find any correlation between the spectrogram and key bits.
Most likely, the time resolution of the spectrogram is too low, to get a good correlation.
The next attempt was done by using a demodulated time series signal.
A new static alignment was implemented, that works on time series signal was implemented.

In addition I experiemented with the loop antenna and replaced the 3 turn with a one turn loop antenna.
I also first made the mistake and did not scan up to the clock frequency of 16 MHz so I missed some good signals
Now there are some good signals around the clock frequency of 16MHz (141MHz in that graph).
Interestingly the second harmonics are also very strong (157MHz).
They are very interesting as they could be analyzed without using an upconverter.

<img class="imgCenter" src="images/arduino-scan-hf.jpg">

I've selected a carrier close to the clock frequency at 16.253MHz, applied a low pass filter with 233kHz bandwidth (70 times lower than clock frequency!) and amplitude demodulated the signal.
The resulting signal has a lot of high frequency components and thus it looks a bit noisy.
Even though after 6022 traces this noise still remains, so it is actually caused by the CPU and could contain sidechannel information.

<img class="imgCenter" src="images/des-mean-osr-666.jpg">

## Breaking DES using CPA
A good method for breaking DES with a power analysis is the so called Correlation Power Analysis.
Its most advantage in comparison to Differential Power Analysis (DPA) is, that we can correlate the power consumption with any arbitrary function.
It makes use of the person correlation which looks as follows:

$$ corr(X,Y) = \frac{n \sum_i x_i y_i - \sum_i x_i \sum_i y_i }{\sqrt{n \sum_i x_i^2 - (\sum_i x_i)^2} \sqrt{n \sum_i y_i^2 - (\sum_i y_i)^2}} $$

This formular looks very complicatie but gives the correct result and we only have to keep track of the number of traces and the sum of $x_i$, $x_i^2$, $x_iy_i$, $y_i$ and $y_i^2$.
The correlation coefficient can then be easily computed at any time using the formular above.

In the case of DES we uses the fact that a XOR operation with 1 (aka bit flip) uses more power than xor with 0.
Depending on the challenge, the Number of bit flips is predicted and directly correlated with the power consumption.
The important part of the DES is the application of the sbox in the first (or last) round.

$$ r = r \oplus sbox_i[l \oplus k_i] $$

$l$ and $r$ are parts of the plaintext and can be controlled by the attacker, $k_i$ is a 6 bit subkey for the $i$-th sbox which is a nonlinear lookup table.
The number of bit flips depends on a part of the plaintext and a part of the key.
We now correlate the hamming weight of the output of the sbox with the actually measured power consumption.

$$ corr(s_t, ham(sbox_i[r \oplus k_i])) $$

This correlation is computed for all possible 64 subkeys and every timestep of the power trace.
For the right subkey, we expect a high correlation with the actual measured sample $s_t$ exactly at the point, where the sbox operation happens.
The following image shows the correlation for the correct subkey of the first sbox (black) and the mean trace (gray) after 6022 traces.
The spike (high correlation) in the beginning of the DES routine is caused by the xor operation with the sbox output.

<img class="imgCenter" src="images/des-corr-osr-666.jpg">

By comparing the maximum correlation over time for all possible subkeys, the correct subkey had the highest correlation after 1500 traces.
The distance between first and second winner is quite significant and a total correlation of 0.1 not too bad, especially as the underlying signal had a 70 times lower bandwidth than the CPU clock of the DUT.

<img class="imgCenter" src="images/des-trend-osr-666.jpg)
