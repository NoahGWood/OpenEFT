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
# And OpenEFT requires OpenJPEG for JP2K support
# Disable OpenJPEG's internal testing and apps to keep the build lean
set(BUILD_CODEC OFF CACHE BOOL "" FORCE)
set(BUILD_THIRDPARTY OFF CACHE BOOL "Build third-party libs" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "NEVER ENABLE THIS" FORCE)
add_subdirectory(vendor/openjpeg)
add_subdirectory(OpenEFT)