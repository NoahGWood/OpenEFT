# Build nbis
mkdir build
./copy_files.sh
./build_nbis.sh
./install_pip.sh
./build_msi.sh
./build_app.sh