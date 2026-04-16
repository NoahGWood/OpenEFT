// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <tiffio.h>
#include <OpenEFT/image/Types.h>
#include <OpenEFT/nist/Helpers.h>

namespace OpenEFT {

    struct MemoryStream {
        const uint8_t* data;
        size_t size;
        size_t offset;
    };

    // ---- TIFF IO callbacks ----
    static tsize_t TiffRead(thandle_t handle, void* buf, tsize_t size) {
        MemoryStream* s = (MemoryStream*)handle;

        size_t remaining = s->size - s->offset;
        size_t to_read = std::min<size_t>(size, remaining);

        memcpy(buf, s->data + s->offset, to_read);
        s->offset += to_read;

        return (tsize_t)to_read;
    }

    static tsize_t TiffWrite(thandle_t, void*, tsize_t) {
        return 0;  // read-only
    }

    static toff_t TiffSeek(thandle_t handle, toff_t off, int whence) {
        MemoryStream* s = (MemoryStream*)handle;

        switch (whence) {
            case SEEK_SET:
                s->offset = off;
                break;
            case SEEK_CUR:
                s->offset += off;
                break;
            case SEEK_END:
                s->offset = s->size + off;
                break;
        }

        return (toff_t)s->offset;
    }

    static int TiffClose(thandle_t) { return 0; }

    static toff_t TiffSize(thandle_t handle) {
        return (toff_t)((MemoryStream*)handle)->size;
    }

    static int TiffMap(thandle_t, void**, toff_t*) { return 0; }

    static void TiffUnmap(thandle_t, void*, toff_t) {}

    // ---- Find TIFF header ----
    static const uint8_t* FindTiffStart(const uint8_t* data, size_t size) {
        for (size_t i = 0; i + 3 < size; ++i) {
            // II*\0
            if (data[i] == 0x49 && data[i + 1] == 0x49 && data[i + 2] == 0x2A &&
                data[i + 3] == 0x00)
                return data + i;

            // MM\0*
            if (data[i] == 0x4D && data[i + 1] == 0x4D && data[i + 2] == 0x00 &&
                data[i + 3] == 0x2A)
                return data + i;
        }
        return nullptr;
    }

    // ---- Main extractor ----
    inline bool ExtractTiffLibtiff(const nnist::Record& record, Image& out) {
        const uint8_t* data =
            reinterpret_cast<const uint8_t*>(record.raw_record.data());
        size_t size = record.raw_record.size();

        const uint8_t* tiff_start = FindTiffStart(data, size);
        if (!tiff_start) {
            out.info.error = "TIFF magic not found";
            return false;
        }

        size_t tiff_size = size - (tiff_start - data);

        MemoryStream stream{tiff_start, tiff_size, 0};

        TIFF* tif =
            TIFFClientOpen("mem", "r", &stream, TiffRead, TiffWrite, TiffSeek,
                           TiffClose, TiffSize, TiffMap, TiffUnmap);

        if (!tif) {
            out.info.error = "TIFF open failed";
            return false;
        }

        uint32_t width = 0, height = 0;
        uint16_t bps = 0, spp = 1;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);

        out.info.width = (int)width;
        out.info.height = (int)height;
        out.info.bit_depth = bps;
        out.info.channels = spp;
        out.info.format = FORMAT::TIFF;
        out.info.valid = (width > 0 && height > 0);

        if (!out.info.valid) {
            TIFFClose(tif);
            out.info.error = "Invalid TIFF dimensions";
            return false;
        }

        // ---- Allocate RGBA (safe universal format) ----
        std::vector<uint32_t> raster(width * height);

        if (!TIFFReadRGBAImageOriented(tif, width, height, raster.data(),
                                       ORIENTATION_TOPLEFT, 0)) {
            TIFFClose(tif);
            out.info.error = "TIFF decode failed";
            return false;
        }

        TIFFClose(tif);

        out.info.channels = 3;  // using RGB for our renderer
        out.info.bit_depth = 8;

        out.pixels.resize(width * height * 3);

        for (size_t i = 0; i < raster.size(); ++i) {
            uint32_t px = raster[i];

            uint8_t r = TIFFGetR(px);
            uint8_t g = TIFFGetG(px);
            uint8_t b = TIFFGetB(px);

            out.pixels[i * 3 + 0] = r;
            out.pixels[i * 3 + 1] = g;
            out.pixels[i * 3 + 2] = b;
        }

        return true;
    }
}  // namespace OpenEFT
