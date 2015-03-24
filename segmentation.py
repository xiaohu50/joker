import sys
from PIL import Image
from numpy import *

def histeq(im, nbr_bins=256):
    imhist, bins = histogram(im.flatten(), nbr_bins, normed=True)
    cdf = imhist.cumsum()
    cdf = 255*cdf / cdf[-1]
    im2=interp(im.flatten(), bins[:-1], cdf)

    return im2.reshape(im.shape), cdf


if __name__=='__main__':
    fin=sys.argv[1]
    fout=sys.argv[2]
    im = Image.open(fin)
    arr_im=array(im)

    ##
    # convert the RGB color data to appropriate data, 
    # gray value is a choice, but not a good one
    ##
    #im_normal=im.convert(mode="P", palette=0)
    #im_normal=im_normal.resize((180,180))
    im_normal=im.convert("L")
    print im_normal.format, im_normal.size, im_normal.mode


    ##
    # histogram equalization maybe is not necessary
    ##
    #arr_im_normal, cdf=histeq(array(im_normal))

    ##
    # segment the picture, and cut the character out
    ##

    ##
    # output the processed picture
    ##
    #im_out=Image.fromarray(uint8(arr_im_normal))
    im_out=im_normal
    print im_out.format, im_out.size, im_out.mode
    im_out.save(fout)
    
    
