from scipy.io.wavfile import read
import numpy as np
import pylab as plt
import top_block as tb

tb.main()
plt.ion()
plt.set_cmap("gray")
a = read("fmdemod.wav")
x = map(float, a[1])

while True:
    rowlen = 41109#int(raw_input("Rowlen: "))
    i = 10000#int(raw_input("Offset: "))
    img = []
    while i+rowlen < len(x):
        img += [x[i:i+rowlen]]
        i += rowlen

    img = np.array(img)
    plt.imshow(img, interpolation='bilinear', aspect='auto')
    plt.draw()
