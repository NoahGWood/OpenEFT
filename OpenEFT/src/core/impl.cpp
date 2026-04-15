#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <OpenEFT/nist/NBISWrapper.h>
extern "C" {
    int debug = 0;  // NIST libraries use this to toggle verbose logging
}
