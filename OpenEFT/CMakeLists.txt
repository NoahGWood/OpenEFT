file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(OpenEFT
    ${SOURCES}
    ${CMAKE_SOURCE_DIR}/vendor/tinyfd/tinyfiledialogs.c
)
target_include_directories(OpenEFT PUBLIC
    include/
    ${CMAKE_SOURCE_DIR}/vendor/tinyfd/
    ${CMAKE_SOURCE_DIR}/vendor/json/single_include/
    ${CMAKE_SOURCE_DIR}/vendor/stb/
    /home/noah/NBIS/include
)
target_compile_options(OpenEFT PRIVATE -Wno-register)
target_link_libraries(OpenEFT PRIVATE
    NMTK::nmtk
    nnist
# TIFF
    tiff
# NBIS libs:
    /home/noah/NBIS/lib/liban2k.a
    /home/noah/NBIS/lib/libcblas.a
    /home/noah/NBIS/lib/libf2c.a
    /home/noah/NBIS/lib/libfft.a
    /home/noah/NBIS/lib/libimage.a
    /home/noah/NBIS/lib/libjpegb.a
    /home/noah/NBIS/lib/libmindtct.a
    /home/noah/NBIS/lib/libnfiq.a
    /home/noah/NBIS/lib/libopenjp2.a
    /home/noah/NBIS/lib/libpcautil.a
    /home/noah/NBIS/lib/libpng.a
    /home/noah/NBIS/lib/libwsq.a
    /home/noah/NBIS/lib/libbozorth3.a
    /home/noah/NBIS/lib/libclapck.a
    /home/noah/NBIS/lib/libfet.a
    /home/noah/NBIS/lib/libihead.a
    /home/noah/NBIS/lib/libioutil.a
    /home/noah/NBIS/lib/libjpegl.a
    /home/noah/NBIS/lib/libmlp.a
    /home/noah/NBIS/lib/libnfseg.a
    /home/noah/NBIS/lib/libpca.a
    /home/noah/NBIS/lib/libpcax.a
    /home/noah/NBIS/lib/libutil.a
    /home/noah/NBIS/lib/libz.a    
)
if(MINGW)
    target_link_options(OpenEFT PRIVATE -mwindows)
endif()