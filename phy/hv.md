## 40kV Cascade

The first high voltage cascade was build by using 10nF 2kV capacitors and a series of 3 1N4007 diodes in series.
It has 20 stages resulting in a maximum output voltage of 20kV.
To ensure an evenen voltage distribution between the diodes, a voltage divider with 10M resistors is added in parallel.
This has also the advantage, that cascade will discharge quickly when it is powerd off.
For every stage corona rings were added, even if they are not necessarry at these voltages.

![alt tag](images/cascade-i-closeup.jpg)

The driver was based using the core of a old flyback transformer, with self winded primary and secondary coils.
It was powered by 2N3055 feedback circuit.

## 200kV Cascade

The second Cascade was build using 20kV 100pF capacitors and 20kV diodes, thus resulting in a maximum output voltage of 200kV.
The first driver was also based on the same circuit as the 40kV one but on high loads, the transistors failed within seconds.
The main problem was probably the too small cooling fins, even though some powerfull discharges could be acchieved.

![alt tag](images/cascade-ii.jpg)

## 200kV Cascade Driver Update

The new driver for the Cascade was based on the ZVS circuit using IRFZ44N which can handle 49A @ 55V.
As the FETs have a RDS of only 17mOhm they dissipate less heat than the 2N3055 transistors.
At 12V the dirver draws up to 11A without any problems.
