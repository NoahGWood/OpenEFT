# OpenEFT
OpenEFT is an open-source project created because the ATF are incompetent at best and maliciously trying to thwart civil rights at worst.

<img src="blurred.png" style="zoom:10%;" />

So, if you're here, you probably need to file a Form 1 and would like to file electronically so that it doesn't take 8 months for the ATF to lose your paper fingerprint card, only to find the fingerprinting companies have jacked their prices up to extortionate amounts.

If you're not up for compiling and installing software to DIY, I have created a generation service that you can use at [OpenEFT.org](https://www.openeft.org/eft)

## How To Use

1. Print off FD-258 card (Use the provided template in the repo for best results)

2. Add fingerprints to form (you don't need to fill out the boxes)

3. Take a high-resolution **1:1** scan or picture of the completed FD-258 card.

4. Run the python program from the command line like so:

   ```bash
   ./eft.py /path/to/your/file.png
   ```

5. Follow the instructions the program provides and respond to the prompts with your info.

6. ??? (programming magic happens here, just relax)

7. Congratulations! You now have an EFT!

## Tips

#### How To Record Fingerprints

If you've never had your fingerprints taken, you definitely need to know how to do it. The first ten prints should be "rolled" on as the video demonstrates, then you need to take two "slap" impressions of the four fingers on each hand and two flat impressions of the thumbs.

<iframe width="560" height="315" src="https://www.youtube.com/embed/47YLGj_yLVA" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>

#### Bad Fingerprints

If you mess up on a fingerprint, you can use a white address label within the blue or black bounding boxes to re-ink that specific finger so you don't need to waste multiple cards. You're not *supposed* to use white out, it could be rejected by the ATF/FBI, but if you're willing to try it let us know if it works! 

## Installation (Linux)

1. Clone the repo:

   ```bash
   git clone https://github.com/NoahGWood/OpenEFT.git
   ```

2. Install requirements:

   ```bash
   pip install -r requirements.txt
   ```

5. Run the program!

   ```bash
   ./OpenEFT.py /path/to/your/tenprint/scan.png
   ```

### Requirements

#### Operating Systems

* Linux Only

#### External Libraries

* [OpenJPEG](https://github.com/uclouvain/openjpeg)

#### Python Modules

* https://github.com/opencv/opencv-python
* [Imutils](https://github.com/PyImageSearch/imutils)
* [Pillow](https://python-pillow.org/)
* [NumPy](https:://numpy.org)
