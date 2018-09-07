---
title: "High Voltage Generation"
description: "A collection of High voltage generators"
---


## 40kV Cascade

<img class="imgRight" src="images/cascade-i-closeup.jpg">

The first high voltage cascade was build by using 10nF 2kV capacitors and a series of 3 1N4007 rectifiers in series.
It has 20 stages resulting in a maximum output voltage of 20kV.
To ensure an evenen voltage distribution between the rectifiers, a voltage divider with 10M resistors is added in parallel.
This has also the advantage, that the cascade will discharge quickly when it is powerd off.
For every stage corona rings were added, even if they are not necessarry at these voltages.


The driver was based using the core of a old flyback transformer, with self winded primary and secondary coils.
It was powered by 2N3055 feedback circuit.

## 200kV Cascade

<img class="imgLeft" src="images/cascade-ii-preassembly.jpg">

The second Cascade was build using 20kV 100pF capacitors and 20kV rectifiers, thus resulting in a maximum output voltage of 200kV.
As un such high voltages, corona discharges become a significant problem, the cascade was encapsulated in epoxy.
The complete assembly was put in a 4cm glass tube and both ends are sealed with acrylic glass.
Terminals for ground and hv output are made from aluminum discs with threads for mounting on the driver base.
To provide the AC to power the cascade, electrical feedthroughs are mad of M4 threaded rod and glass tubings.


As I did not provide any cooling a lots of bubbles build up in the epoxy and the galass tube cracked.
This is no practical issue and gives the cascade an interesting look.

<img class="imgRight" src="images/cascade-ii.jpg">

The first driver was based on the same circuit as the 40kV one but on full load, the transistors failed within seconds.
After a friend of was laughing at me for the small cooling fins and after some explanations, I realized, that the transistors are mostly driven in in linear mode, where the are regulating the current.
As they are not switching, a lot of power was disipated on them.


## 200kV Cascade Driver Update

<img class="imgLeft" src="images/zvs.jpg">

The new driver for the Cascade was based on the ZVS circuit using IRFZ44N which can handle 49A @ 55V.
As the FETs have a RDS of only 17mOhm they dissipate less heat than the 2N3055 transistors.
At 15V the dirver draws up to 10A without any problems.

## Xrays from Rectifier Tube

<img class="imgRight" src="images/rectifier-xray.jpg">

Instead of building a X-Ray tube using a high vacuum pump, we can also use high voltage rectifier tubes as they were used in very old televisions.
The top terminal is insulated enough, to apply voltage much higher than the recommended 20kV.
Even though if the filament is provided with full power, the current through the tube is too high so the applied voltage will drop so that no X-Rays are generated.
Using a lab power supply to adjust the filament voltage was not to be a good idea, as the tube current is not constant the electronics goes crazy if it's hit by high voltage spikes.
This results in random action by the power supply which can easily damage the tube.
Instead a small transformer and a couple of resistors were used.
If everything is setup correctly, the glass will fluorescent as it is hit by X-Rays.
As it's not a good idea to stand next to the tube, while it is running, a camera was set up with an exposure time of 60sec and the high voltage was switched on from behind a concrete wall.
Even though I measured a dose rates of 3-5 uSv/h what is roughly 30 time background and comparable to what you will get in a plane.


