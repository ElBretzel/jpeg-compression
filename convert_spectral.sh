#!/bin/sh

# Convert image to ppm so cjpeg can convert to a supported format the program can decode
magick "$1" original.ppm

# Disable subsampling + using spectral only progressive mode
cjpeg -sample 1x1 -progressive -scans scans.txt original.ppm > converted.jpg

# Decode back to ppm
./../build/a.out "./converted.jpg"

