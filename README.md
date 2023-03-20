# OpenEFT
OpenEFT is an open-source project created because the ATF are incompetent at best and maliciously trying to thwart civil rights at worst.

Like the project? Consider sponsoring me so I can continue to develop these free tools!

[![GitHub Sponsor](https://img.shields.io/github/sponsors/NoahGWood?label=Sponsor&logo=GitHub)](https://github.com/sponsors/NoahGWood)

<img src="docs/imgs/blurred.png" width="250" />

So, if you're here, you probably need to file a Form 1 and would like to file electronically so that it doesn't take 8 months for the ATF to lose your paper fingerprint card, only to find the fingerprinting companies have jacked their prices up to extortionate amounts.

If you're not up for compiling and installing software to DIY, I have created a generation service that you can use at [OpenEFT.org](https://www.openeft.org/eft)

## How To Use

1. Print off FD-258 card (Use the provided template in the repo for best results)

2. Add fingerprints to form (you don't need to fill out the boxes)

3. Take a high-resolution **1:1** scan or picture of the completed FD-258 card.

4. Run the python program from the command line like so:

   ```bash
   python3 openeft.py
   ```

5. Follow the instructions the program provides and respond to the prompts with your info.

6. ??? (programming magic happens here, just relax)

7. Congratulations! You now have an EFT!

## Tips

#### How To Record Fingerprints

If you've never had your fingerprints taken, you definitely need to know how to do it. The first ten prints should be "rolled" on as the video demonstrates, then you need to take two "slap" impressions of the four fingers on each hand and two flat impressions of the thumbs.

[![Everything Is AWESOME](http://img.youtube.com/vi/47YLGj_yLVA/0.jpg)](//www.youtube.com/watch?v=47YLGj_yLVA "Fingerprinting Technique")

#### Bad Fingerprints

If you mess up on a fingerprint, you can use a white address label within the blue or black bounding boxes to re-ink that specific finger so you don't need to waste multiple cards. You're not *supposed* to use white out, it could be rejected by the ATF/FBI, but if you're willing to try it let us know if it works! 

## Installing

1. Clone the repo:

   ```sh
   git clone --recurse-submodules https://github.com/NoahGWood/OpenEFT.git
   mkdir build
   cd nbis
   # If Windows
   # If python3 not installed
   python3
   # If WSL not installed, get privileged shell
   powershell -Command "Start-Process cmd -Verb RunAs"
   # In privileged shell
   wsl --install -d Ubuntu
   exit
   # In normal shel
   ./setup ../build --MSYS [--32 | --64]
   make config
   make it
   make install
   cd ../
   echo 'export PATH="$PWD/build/bin:$PATH" > ~/.bashrc
   cd browser
   wget "https://github.com/ungoogled-software/ungoogled-chromium-windows/releases/download/111.0.5563.65-1.1/ungoogled-chromium_111.0.5563.65-1.1_windows_x64.zip"
   unzip ungoogled-chromium_111.0.5563.65-1.1_windows_x64.zip
   mv ungoogled-chromium_111.0.5563.65-1.1_windows windows
   # If Linux
   ./setup.sh ../build/
   make config
   make it
   make install LIBBNIS=no
   cd ../
   echo 'export PATH="$PWD/build/bin:$PATH" > ~/.bashrc
   mkdir browser
   cd browser
   wget "https://github.com/clickot/ungoogled-chromium-binaries/releases/download/111.0.5563.65-1/ungoogled-chromium_111.0.5563.65-1.1.AppImage"
   mkdir linux
   mv ungoogled-chromium_111.0.5563.65-1.1.AppImage linux/chrome.AppImage
   # Finally
   cd ../
   ```

2. Install requirements:

   ```sh
   pip install -r requirements.txt
   ```

3. Install libopenjp2:

   ```bash
   sudo apt install libopenjp2-tools
   ```

5. Run the program!

   ```bash
   python3 openeft.py

## Requirements

#### Operating Systems

* Linux Only (maybe, I have no idea tbh.)

#### External Libraries

* [OpenJPEG](https://github.com/uclouvain/openjpeg)
* [NBIS](https://www.nist.gov/services-resources/software/nist-biometric-image-software-nbis) (Need to build from scratch for now until a proper installer is created)

#### Python Modules

* [OpenCV-Python](https://github.com/opencv/opencv-python)
* [Imutils](https://github.com/PyImageSearch/imutils)
* [Pillow](https://python-pillow.org/)
* [NumPy](https:://numpy.org)
* django

## ScreenShots

![](docs/imgs/openeft_gui.png)

![Confirmation With ATF](docs/imgs/personal.png)

![](docs/imgs/eft_files.png)



![Confirmation With ATF](docs/imgs/ITWORKS.png)
