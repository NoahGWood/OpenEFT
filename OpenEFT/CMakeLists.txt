# Copyright (c) 2026 Noah G. Wood
# SPDX-License-Identifier: LicenseRef-OpenEFT
#
# This file is part of OpenEFT.
# See LICENSE for details.

file(GLOB_RECURSE SOURCES src/*.cpp)
# NBIS libs
# Keeping separate for now just in case we need other libs later
# Some of the libs just won't work on windows
if(MINGW OR WIN32)
    set(NBIS_ROOT ${CMAKE_SOURCE_DIR}/vendor/NBIS/artifacts/windows)
    set(NBIS_LIBS
        ${NBIS_ROOT}/lib/libwsq.a
        ${NBIS_ROOT}/lib/libimage.a
        ${NBIS_ROOT}/lib/libihead.a
        ${NBIS_ROOT}/lib/libioutil.a
        ${NBIS_ROOT}/lib/libjpegl.a
        ${NBIS_ROOT}/lib/libfet.a
        ${NBIS_ROOT}/lib/libutil.a
    )
else()
    set(NBIS_ROOT ${CMAKE_SOURCE_DIR}/vendor/NBIS/artifacts/linux)
    set(NBIS_LIBS 
        ${NBIS_ROOT}/lib/libwsq.a
        ${NBIS_ROOT}/lib/libimage.a
        ${NBIS_ROOT}/lib/libihead.a
        ${NBIS_ROOT}/lib/libioutil.a
        ${NBIS_ROOT}/lib/libjpegl.a
        ${NBIS_ROOT}/lib/libfet.a
        ${NBIS_ROOT}/lib/libutil.a
        ${NBIS_ROOT}/lib/libpcautil.a
        ${NBIS_ROOT}/lib/libpca.a
    )
endif()
# Have to set this because OpenJPEG is doing some weird shit idfk
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(OpenEFT
    ${SOURCES}
    ${CMAKE_SOURCE_DIR}/vendor/tinyfd/tinyfiledialogs.c
)
target_compile_definitions(OpenEFT PRIVATE __NBISLE__)
target_include_directories(OpenEFT PUBLIC
    include/
    ${CMAKE_SOURCE_DIR}/vendor/tinyfd/
    ${CMAKE_SOURCE_DIR}/vendor/json/single_include/
    ${CMAKE_SOURCE_DIR}/vendor/stb/
    ${CMAKE_SOURCE_DIR}/vendor/openjpeg/src/lib/
    ${NBIS_ROOT}/include
)
target_compile_options(OpenEFT PRIVATE -Wno-register)
target_link_libraries(OpenEFT PRIVATE
    NMTK::nmtk
    nnist
# TIFF
    tiff
# OpenJPEG
    openjp2
# NBIS libs
    ${NBIS_LIBS}
)
if(MINGW OR WIN32)
    target_include_directories(OpenEFT PUBLIC
        ${CMAKE_SOURCE_DIR}/vendor/NBIS/compat/
    )
    target_link_options(OpenEFT PRIVATE -mwindows)
    target_link_options(OpenEFT PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
    target_compile_definitions(OpenEFT PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()