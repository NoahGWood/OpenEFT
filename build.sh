set -e

LINUX_BUILD="build-linux"
WIN_BUILD="build-mingw"

echo "==> Building for Linux..."
cmake -S . -B $LINUX_BUILD -G Ninja -DCMAKE_BUILD_TYPE=Release 
cmake --build $LINUX_BUILD --config Release
mkdir -p release/linux
cp $LINUX_BUILD/bin/OpenEFT release/linux/OpenEFT
./release/linux/OpenEFT


echo "==> Building for Windows..."
cmake -S . -B $WIN_BUILD -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw64.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build $WIN_BUILD
mkdir -p release/windows
cp $WIN_BUILD/bin/OpenEFT.exe release/windows/OpenEFT.exe
wine release/windows/OpenEFT.exe

# echo "==> Checking formatting..."
# tools/format_check.sh

# echo "==> Running static analysis (cert)..."
# tools/lint_cert.sh build

# ./build-linux/OpenEFT/OpenEFT
# wine build-mingw/OpenEFT/OpenEFT