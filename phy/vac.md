# Diffusion Pump

A few years ago, i was able to get a cheap refurbished diffusion pump on eBay, which can generate a very high vacuum.
To test the Pump for the first time, some additional parts are required, such as a rotary vacuum pump and some vacuum fittings.
As the pump needs water cooling, a small aquarium pump and a large cup of water was used.
Using a filament from a halogen lamp and a high voltage power supply, a simple diode can be build generating a massive amount of X-rays.
Electrons are emitted from the glowing tungsten wire and are accelerated to the positive plate.
If the vacuum is good enough that the electrons are not slowed down by remaining gas, the assembly should be able to generate some X-rays.
The following foto was taken using a delayed shutter and the high voltage supply was switched on from a safe distance of a couple of meters behind a concrete wall to avoid unnecessary irradiation.
The Geiger counter shows a dose rate of 1.5 mSv/h a couple of centimeters away.

![alt tag](images/xray.jpg)

The difference between a low pressure and a high vacuum discharge is shown in the following.
These Pictures were made using glass tube with 4cm diameter, that can be evacuated.
A high positive voltage was applied on the top using the 40kV cascade, whereas the bottom copper tube is grounded.
On the left hand side the diffusion pump is switched off, and the typical cathod rays and positive column are clearly visible.
With the diffusion (right), there are not enough gas molecules left for a gas discharge.
Instead arcing takes place on the inside of the glass tube, due to impurities on the surface.
In addition a small amount of X-rays are generated.

![alt tag](images/disch.jpg)

As the pump was not in used for several years, the first startup was a little bit problematic.
No matter what we tried, the diffusion pump did created a high vacuum and we still had a gas discharge.
We noticed that by just using the backing pump we only achieved a couple of hundreds volt for a gas discharge.
By using [Paschen's law](https://en.wikipedia.org/wiki/Paschen%27s_law#/media/File:Paschen_curves.svg), we eyeballed a pressure of ca. 1mBar, but the diffusion pump requires a preassure of at least 0.4mBar.
The copper pipe seals seem not to be ideal for vacuum use, as the tend to turn a little bit if some force is applied and they become leaky after some time.
This problem could be solved by simplifying the hub and remove all of those flanges.

![alt tag](images/badseals.jpg)

In the end we noticed, that the problem could be identified earlier by just comparing the color of the gas discharges.
For a leaky pump a lot of air is present in the chamber and therefore a an slightly orange/blue discharge can be observed.
After fixing the issue the gas discharge turned to white.
From left to right:
    - Leaky backing pump
    - Fixed backing pump (comparable to leaky pump + diffusion pump)
    - Fixed backing pump + diffusion pump

![alt tag](images/disch-comparison.jpg)


## Iongun

As a long term goal for the diffusion pump would be nuclear fusion, and therefore an iongun is one important step for this.
The principle is simple, a glass tube is mounted on a KF flange, with a small hole for the ions to pass through.
In the glass tube itself is a slightly higher pressure, that allow for a gas discharge and therefore to build up free ions.
An external electrical field will be applied to accelerate the ions towards the hole, where some will pass through into the high vacuum chamber.
I needed several attempts to make the assembly airtight.
First I tried to use vacuum grease but in the end I noticed that the holes in the KF flange are slightly too big, so I ended up using epoxy.
The gas is feeded through a copper capillary and a small hole on the side of the tube mount.
The pressure can be regulated with a needle valve, what usually is used for CO2 applications.


![alt tag](images/iongun.jpg)

The first test were very promising, whereas the vacuum wasn't ideal.
In fact I wasn't able to open the valve even a bit as the pressure would increase too much.
The main problem was most likely, that the vacuum fittings were slightly dirty or had minor scratches, what could be fixed using vacuum grease.
Event though anode rays are clearly visible.

![alt tag](images/ionray.jpg)
