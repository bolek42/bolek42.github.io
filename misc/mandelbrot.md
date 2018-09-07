---
title: "4D Mandelbrot"
description: "A 4D mandelbrot implementation using arbitrary start values"
---

# The Normal Mandelbrot
The Mandeblort figure ist computed by the following series, wheres the real part of c is the x and the imaginary part the y axis of the plot.
This series can either converge or diverge.
It can be shown, that if a intermediate value is grater then 2, the series will diverge.
The colour indicates, how many steps are required, until divergence can be detected.

$$ z_n=z_{n-1}^2 + c $$

# Manipulating z0
For most mandelbrot figures, the start value $z_0$ is set to 0.
But its not clear, if this has a non trivial impact (scaling, shift, rotate) to the overall image.
So i wrote a mandelbrot program based on OpenCL and added some keymappings to manipulate $z_0$.
The program will interpolate between two set of parameters and render the image as ppm images, which can then be converted to a video.

Sweeping the real part of $z_0$ from -1.6 to 1.6
<video width="100%" controls>
  <source src="vid/z0r-1.6.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

<video width="100%" controls>
  <source src="vid/z0r-closeup.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

Sweeping the imaginary part of $z_0$ from -1.2 to 1.2
<video width="100%" controls>
  <source src="vid/z0i-1.2.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

<video width="100%" controls>
  <source src="vid/z0i-closeup.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

# Rotating
Lets call $c.i$ the imaginary and $c.r$ the real part of $c$.
A Point in the 4-D Mandelbrot space including the $z_0$ starting value can then be written as $(c.r, c.i, z_0.r, z_0.i)$.
A normal mandelbrot will use the direction vector (1,0,0,0) for the X axis and (0,1,0,0) for the Y axis.
Instead of viewing the mandelbrot in the $c.r$ $c.i$ plane, we can also try to rotate this plane.
In this case starting from a fixed point, I used $X=(cos(\phi),0,sin(\phi),0)$ and $Y=(0,cos(\phi),0,sin(\phi))$ to drwa the mandelbrot image.

Sweeping $\phi$ from 0 to $\pi$ with $(1,0,0,0)$ as start:
<video width="100%" controls>
  <source src="vid/phi-offcenter.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

# Implementation
The Implementation is using OpenCL for computing the mandelbrot values and GLUT for drawing the image ([source](mandelbrot-4d.tar.gz))
The following key mappings are used to controll the Mandelbrot:


| Key    | | Meaning                                                        |
|--------|-|----------------------------------------------------------------|
| Arrows | | Move in X/Y Plane                                              |
| 1/q    | | Modify c.r                                                     |
| 2/w    | | Modify c.i                                                     |
| 3/e    | | Modify z0.r                                                    |
| 4/r    | | Modify z0.i                                                    |
| 5/t    | | Modify $\phi$                                                  |
| 7/u    | | Modify Max Steps                                               |
| +/-    | | Zoom                                                           |
| k      | | Store Parameters                                               |
| 0      | | Reset to last stored position                                  |
| .      | | Render From last stored position                               |
| ,      | | Render From last stored position and store images in /tmp/out  |
| ;      | | Screenshot                                                     |
