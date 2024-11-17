#!/usr/bin/bash

mkdir build
mkdir -p browser/linux

echo "Setting Up NBIS"
sudo rm -rf ./nbis #Remove any old copies
git clone https://github.com/OwenPemberton/nbis ./nbis
cd nbis
./setup.sh ${PWD}/../build --64
echo "Configuring NBIS"
make config
echo "Making NBIS"
make it
echo "Installing NBIS"
sudo make install LIBNBIS=no

echo "Checking NBIS binaries paths"
case :$PATH:
  in *:$HOME/OpenEFT/nbis/nfseg/bin:*) ;; # do nothing, it's there
     *) echo 'export PATH="$HOME/OpenEFT/nbis/nfseg/bin:$PATH"' >> ~/.bashrc #it's missing, add to PATH
esac

case :$PATH:
  in *:$HOME/OpenEFT/nbis/nfiq/bin:*) ;; # do nothing, it's there
     *) echo 'export PATH="$HOME/OpenEFT/nbis/nfiq/bin:$PATH"' >> ~/.bashrc #it's missing, add to PATH
esac

#source ~/.bashrc doesn't work in .sh in Ubuntu since scripts run in their own session. Need to run post build OR run the script with 'source'
source ~/.bashrc

echo "Installing UnGoogled Chromium"
cd ../browser/linux
wget "https://github.com/clickot/ungoogled-chromium-binaries/releases/download/111.0.5563.65-1/ungoogled-chromium_111.0.5563.65-1.1.AppImage"
mv ungoogled-chromium_111.0.5563.65-1.1.AppImage chrome.AppImage
chmod +x chrome.AppImage
cd ../../

echo "Installing LibOpenJP2-Tools"
sudo apt-get install libopenjp2-tools -y

echo "Installing Pip Requirements"
cd ~/OpenEFT
pip3 install -r requirements.txt

python3 manage.py migrate

echo "OpenEFT build complete. Run the app by typing 'python3 openeft.py'"
cd ~/OpenEFT
