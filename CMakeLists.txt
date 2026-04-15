cmake_minimum_required(VERSION 3.16)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
project(OpenEFT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
enable_language(C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# OpenEFT depends on NMTK (Noah's Multiplatform Toolkit)
add_subdirectory(vendor/NMTK)

set(NNIST_BUILD_TESTS OFF) # Requires importing NTest and NCLI to vendor/
# OpenEFT depends on NNIST (Noah's ANSI/NIST Implementation)
add_subdirectory(vendor/NNIST)
# OpenEFT apparently ALSO depends on libtiff because we need to support
# facisimile image extraction...
add_subdirectory(vendor/libtiff)
add_subdirectory(OpenEFT)