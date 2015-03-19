from PIL import Image
from pylab import *
import os
import sys

pin=sys.argv[1]
im = Image.open(pin)
arim=array(im)
print arim.shape, arim.dtype

imshow(arim)
print 'please click 3 points'
x=ginput(3)
print 'you clicked:',x

show()
