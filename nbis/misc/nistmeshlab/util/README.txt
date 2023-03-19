PointClound Creator 
===================

Author: Kenneth Ko
Date:   06/09/2011


Description
-----------
The PointCloud Creator application allows the user to convert a
RAW fingerprint image to the NIST-specific 3D PointCloud image format.
During this process, the PoinCloud Creator will convert the grascale
values in the original RAW file to depth values in the NIST-specific 
32 PointCloud image format (*.XYZ).


Building
--------
Please follow the step below to build:
> gcc pointcloudCreator.c -o pointcloudCreator


Usage 
-----
> Usage: ./pointcloudCreator  <input file> <output file> width height [0|1]
           infile      The RAW gray image file to be read.
           outfile     The XYZ formated file to be written.
           width       The width of the RAW image.
           height      The height of the RAW image.
           gray        With grayscale value in the XYZ (Yes/No)
                       Yes: Set to 1
		       No: Set to 0.


Example
-------
Create a *.XYZ gray fingerprint image from a *RAW fingerprint image.
The examples image can be found in the util/example directory. 

> ./pointcloudCreator nistFinger_500_500.raw finger_gray.xyz 500 500 1


NIST-Specific 3D PointCloud Image Format
----------------------------------------

Point Cloud File format
-----------------------
Comment                         : AN
Type of image format		: xyz | xyz_g | xyz_rgb
Scanner Make and Model		: AN
Scanner Serial Number		: AN
Capture Time and Date		: AN
Capture Location		: AN
Scanner X resolution		: N
Scanner Y resolution		: N
Resolution unit			: A
Scan Orientation to Target	: N

 
The X, Y and Z vertex		: (F F F | N | N N N)*              
 
N = Numeric; A = Alphabetic; AN = Alphanumeric; F = Floating Point
 
 
Example: Point Cloud with xyz
-----------------------------
#Created by PointClound Creator - Version 1.O
#xyz
#ABC Scanner
#SN010101010
#07-07-2010 12:12:12PM
#1000
#1000
#mm
#90
5.135754   -8.172974   5.437447
4.940079   -8.253554   4.630594
4.845107   -8.292492   4.226524
4.988174   -8.216766   3.827115
4.979454   -8.214173   3.425456
4.815156   -8.286430   3.020466
4.936698   -8.221167   2.620421
5.015956   -8.176156   2.219661
5.073014   -8.141862   1.819829
5.298374   -8.026543   1.421424
    .           .          .
    .           .          .
    .           .          .
    .           .          .
    .           .          .
    .           .          .
 
 
Example: Point Cloud with xyz_g
-------------------------------
#xyz_g
#ABC Scanner
#SN010101010
#07-07-2010 12:12:12PM
#1000
#1000
#mm
#90
5.135754 -8.172974 5.437447 123
4.940079 -8.253554 4.630594 255
4.845107 -8.292492 4.226524 244
4.988174 -8.216766 3.827115 212
4.979454 -8.214173 3.425456 234
4.815156 -8.286430 3.020466 127
4.936698 -8.221162 2.620421 145
5.015956 -8.176156 2.219661 231
5.073014 -8.141862 1.819829 123
5.298374 -8.026543 1.421424 123
    .         .        .     .
    .         .        .     .
    .         .        .     .
    .         .        .     .
    .         .        .     .
    .         .        .     .
 
 
Example: Point Cloud with xyz_rgb
---------------------------------
#xyz_rgb
#ABC Scanner
#SN010101010
#07-07-2010 12:12:12PM
#1000
#1000
#mm
#90
5.135754 -8.172974 5.437447 123 223 233
4.940079 -8.253554 4.630594 255 223 233
4.845107 -8.292492 4.226524 244 223 233
4.988174 -8.216766 3.827115 212 223 233
4.979454 -8.214173 3.425456 234 223 233
4.815156 -8.286430 3.020466 127 223 233
4.936698 -8.221162 2.620421 145 223 233
5.015956 -8.176156 2.219661 231 223 233
5.073014 -8.141862 1.819829 123 223 233
5.298374 -8.026543 1.421424 123 223 233
    .          .        .    .   .   .
    .          .        .    .   .   .
    .          .        .    .   .   .
    .          .        .    .   .   .
    .          .        .    .   .   .
    .          .        .    .   .   .


