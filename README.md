# jpeg-compression
We are trying to recreate JPEG image compression format for educational purpose in C++

To compile, cmake -B build
Then main is in build/a.out

Some demo jpeg files are located in demo/

Usage: ./a.out <path>

Generated ppm (jpeg decoded) is in the folder where <path> was described.

Bonus:
If you want to use our decoder with any image, use the ./convert_spectral.sh script
Usage: ./convert_spectral.sh <path>
(Will generate a progressive sequential jpeg)
