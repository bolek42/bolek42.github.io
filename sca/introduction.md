<script src="js/w3.js"></script>
<div w3-include-html="bar.html"></div>
<script>w3.includeHTML();</script>

## Abstract

Sidechannels are uninteded information leakage of an implementation.


## Electrical Emmanation Attacks

All electronic devices create some sort of electrical emmanations, that are caused du to voltage and current fluctuations.
These can be measured either using a digital storage oscilloscope or in this case with an SDR receiver.
An SDR has the advantage of built in analog preamplification and filtering, which can be adjusted from software.
In addition many DSP frameworks are available, that already support SDRs, such as GNURadio.

## Origin of Sidechannel Effects

Most digital circuits are realized as CMOS gates with an positive and negative transistor group.
For each gate output, either the positive or negative group is connecting the output of the gate to VCC or ground.
With a static view of such a gate, there should be no current flowing through the gate.
Even though if a gate output changes its value and the voltage of the output changes, the tiny capacitance of the lines has to be charged or discharged resulting in a current.
This leads to the Hamming-Distance model of Power consumption, that the powerconsumption of CMOS gate is proportional to the number of flipping bits.


## [Desktop PC](sca/openssl.md)

Even desktop PCs and Laptops emmits measurable sidechannel effects, that might be exploitable as Genkin et.al. showed.
These are mostly caused by the CPU voltage regulation, that stabilizes the high frequency fluctuations of the CPU power consumption using inductiors and capacitors and emmiting low frequency signals, that are correlating with individual operations on the CPU.
The following image shows, the setup for sidechannel analysis of a desktop PC:

![alt tag](images/setup-pc.jpg)

The desktop PC on the left is the Device Under Test (DUT) with a loop antenna on top, that gets 'attacked' by the laptop on the right
In the middle, the upconverter and SDR receiver is located.

A simple Experiment to test for basic sidechannel effects to use the following programm, that puts load on one CPU core and then sleeps for a while.

```python
import time
while 1:
    for i in xrange(40000000): pass
    time.sleep(1)
```
Basic Programs such as 'uhd_fft' can be used to visualize the RF spectrum.
There are obvious differences in the spectrogram of an idle or busy CPU.

![alt tag](images/idle-busy.jpg)

Further analysis of the Dell Optiplex showed an very interesting form of side-channel leakage.
The following spectrogram shows OpenSSL multiplications encapsulated by dummy operations (tight for loop)
It seems that the sidechannel effects seems to be frequency modulated on some carrieres.

![alt tag](images/dell-mul-raw.jpg)

## [Arduino](arduino.md)

Also embedded devices such as an Arduino show sidechannel effects.
Similar to the desktop PC setup, a small loop antenna was used to pick up such signals

![alt tag](images/setup-arduino.jpg)
