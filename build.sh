set -e

LINUX_BUILD="build-linux"
WIN_BUILD="build-mingw"

echo "==> Building for Linux..."
cmake -S . -B $LINUX_BUILD -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build $LINUX_BUILD --config Release
./build-linux/OpenEFT/OpenEFT
# cmake --install $LINUX_BUILD --prefix release/linux/OpenEFT

# echo "==> Building for Windows..."
# cmake -S . -B $WIN_BUILD -G Ninja \
#   -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw64.cmake \
#   -DCMAKE_BUILD_TYPE=Release

# cmake --build $WIN_BUILD
# cmake --install $WIN_BUILD --prefix release/windows/OpenEFT

# echo "==> Bundling MinGW runtime DLLs..."

# DLLSRC="/usr/x86_64-w64-mingw32/lib"

# cp "$DLLSRC/libwinpthread-1.dll" release/windows/OpenEFT/bin/ || true
# cp "$DLLSRC/libgcc_s_seh-1.dll" release/windows/OpenEFT/bin/ || true
# cp "$DLLSRC/libstdc++-6.dll" release/windows/OpenEFT/bin/ || true

# echo "==> Checking formatting..."
# tools/format_check.sh

# echo "==> Running static analysis (cert)..."
# tools/lint_cert.sh build

# ./build-linux/OpenEFT/OpenEFT
# wine build-mingw/OpenEFT/OpenEFT