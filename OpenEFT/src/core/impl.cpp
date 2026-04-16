// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <OpenEFT/nist/NBISWrapper.h>
extern "C" {
    int debug = 0;  // NIST libraries use this to toggle verbose logging
}
