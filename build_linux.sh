#!/usr/bin/bash

mkdir build
mkdir -p browser/linux
echo "Updating Submodules"
git submodule update
echo "Setting Up NBIS"
cd nbis
./setup.sh ${PWD}/../build --64
echo "Configuring NBIS"
make config
echo "Making NBIS"
make it
echo "Installing NBIS"
make install
echo "Adding NBIS binaries to path"
echo 'export PATH="$PWD/build/bin:$PATH" > ~/.bashrc'
source ~/.bashrc
cd ../browser/linux
 wget "https://github.com/clickot/ungoogled-chromium-binaries/releases/download/111.0.5563.65-1/ungoogled-chromium_111.0.5563.65-1.1.AppImage"
mv ungoogled-chromium_111.0.5563.65-1.1.AppImage chrome.AppImage
cd ../../
echo "Installing Pip Requirements"
pip3 install -r requirements.txt
echo "Installing LibOpenJP2-Tools"
if [ ""$EUID"" != 0 ]; then
    sudo apt-get install libopenjp2-tools
fi
echo "Starting OpenEFT"
python3 openeft.py
