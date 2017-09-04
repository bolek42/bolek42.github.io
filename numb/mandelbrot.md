
# The Normal Mandelbrot
The Mandeblort figure ist computed by the following series, wheres the real part of c is the x and the imaginary part the y axis of the plot.
This series can either converge or diverge.
It can be shown, that if a intermediate value is grater then 2, the series will diverge.
The collor indicates, how many steps are required, until divergence can be detected.

<a href="https://www.codecogs.com/eqnedit.php?latex=z_n=z_{n-1}^2&space;&plus;&space;c" target="_blank"><img src="https://latex.codecogs.com/gif.latex?z_n=z_{n-1}^2&space;&plus;&space;c" title="z_n=z_{n-1}^2 + c" /></a>

# Manipulating z0

For most mandelbrot figures, the start value is set to 0.
But its not clear, if this has a non trivial impact (scaling, shift, rotate) to the overall image.

Sweeping the real part of z0 from -1.6 to 1.6
<video width="100%" controls>
  <source src="vid/z0r-1.6.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

<video width="100%" controls>
  <source src="vid/z0r-closeup.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

Sweeping the imaginary part of z0 from -1.2 to 1.2
<video width="100%" controls>
  <source src="vid/z0i-1.2.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

<video width="100%" controls>
  <source src="vid/z0i-closeup.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

