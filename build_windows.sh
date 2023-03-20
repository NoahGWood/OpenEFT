#!/usr/bin/bash
#wsl --install -d Ubuntu

mkdir build
mkdir browser/windows
echo "Updating Submodules"
git submodule update
echo "Setting Up NBIS"
cd nbis
./setup.sh ${PWD}/../build --MSYS --64

echo "Configuring NBIS"
make config
echo "Making NBIS"
make it
echo "Installing NBIS"
make install
echo "Adding NBIS binaries to path"
echo 'export PATH="$PWD/build/bin:$PATH" > ~/.bashrc'
source ~/.bashrc
cd ../browser/windows
wget "https://github.com/ungoogled-software/ungoogled-chromium-windows/releases/download/111.0.5563.65-1.1/ungoogled-chromium_111.0.5563.65-1.1_windows_x64.zip"
unzip ungoogled-chromium_111.0.5563.65-1.1_windows_x64.zip
mv ungoogled-chromium_111.0.5563.65-1.1_windows windows
cd ../../
echo "Installing Pip Requirements"
pip3 install -r requirements.txt
echo "Installing LibOpenJP2-Tools"
if [ ""$EUID"" != 0 ]; then
    wsl sudo apt-get install libopenjp2-tools
fi
echo "Starting OpenEFT"
echo 'export PATH="$PWD/build/bin:$PATH" > ~/.bashrc
python3 openeft.py

