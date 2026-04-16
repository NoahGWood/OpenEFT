// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/image/Types.h>
#include <cstring>
#include <vector>

extern "C" {
    #include <openjp2/openjpeg.h>
}
namespace OpenEFT {

    // ---- Memory stream for OpenJPEG ----
    struct JP2KStream {
        const uint8_t* data;
        size_t size;
        size_t offset;
    };

    static OPJ_SIZE_T JP2KRead(void* p_buffer, OPJ_SIZE_T p_nb_bytes,
                               void* p_user_data) {
        JP2KStream* s = (JP2KStream*)p_user_data;

        size_t remaining = s->size - s->offset;
        size_t to_read = std::min<size_t>(p_nb_bytes, remaining);

        memcpy(p_buffer, s->data + s->offset, to_read);
        s->offset += to_read;

        return to_read;
    }

    static OPJ_OFF_T JP2KSkip(OPJ_OFF_T p_nb_bytes, void* p_user_data) {
        JP2KStream* s = (JP2KStream*)p_user_data;
        s->offset += p_nb_bytes;
        return p_nb_bytes;
    }

    static OPJ_BOOL JP2KSeek(OPJ_OFF_T p_nb_bytes, void* p_user_data) {
        JP2KStream* s = (JP2KStream*)p_user_data;
        if (p_nb_bytes < 0 || (size_t)p_nb_bytes > s->size)
            return OPJ_FALSE;

        s->offset = (size_t)p_nb_bytes;
        return OPJ_TRUE;
    }

    // ---- Find JP2K start ----
    inline const uint8_t* FindJP2KStart(const uint8_t* data, size_t size) {
        for (size_t i = 0; i + 4 < size; ++i) {
            // JP2 signature box
            if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x00 &&
                data[i + 3] == 0x0C) {
                return data + i;
            }

            // J2K codestream (FF4F)
            if (data[i] == 0xFF && data[i + 1] == 0x4F) {
                return data + i;
            }
        }
        return nullptr;
    }

    // ---- Main decoder ----
    inline static bool DecodeJP2K(const uint8_t* data, size_t size, Image& out) {
        const uint8_t* start = FindJP2KStart(data, size);
        if (!start) {
            out.info.error = "JP2K magic not found";
            out.info.valid = false;
            return false;
        }

        size_t actual_size = size - (start - data);

        JP2KStream stream{start, actual_size, 0};

        opj_dparameters_t params;
        opj_set_default_decoder_parameters(&params);

        // Try JP2 first, fallback to J2K
        opj_codec_t* codec = opj_create_decompress(OPJ_CODEC_JP2);
        opj_setup_decoder(codec, &params);

        opj_stream_t* opj_stream = opj_stream_create(1024, OPJ_TRUE);
        opj_stream_set_user_data(opj_stream, &stream, nullptr);
        opj_stream_set_user_data_length(opj_stream, stream.size);
        opj_stream_set_read_function(opj_stream, JP2KRead);
        opj_stream_set_skip_function(opj_stream, JP2KSkip);
        opj_stream_set_seek_function(opj_stream, JP2KSeek);

        opj_image_t* image = nullptr;

        if (!opj_read_header(opj_stream, codec, &image)) {
            // Retry as raw codestream
            opj_destroy_codec(codec);

            codec = opj_create_decompress(OPJ_CODEC_J2K);
            opj_setup_decoder(codec, &params);

            stream.offset = 0;
            opj_stream_set_user_data(opj_stream, &stream, nullptr);

            if (!opj_read_header(opj_stream, codec, &image)) {
                out.info.error = "JP2K header read failed";
                out.info.valid = false;
                opj_stream_destroy(opj_stream);
                opj_destroy_codec(codec);
                return false;
            }
        }

        if (!opj_decode(codec, opj_stream, image)) {
            out.info.error = "JP2K decode failed";
            out.info.valid = false;
            opj_image_destroy(image);
            opj_stream_destroy(opj_stream);
            opj_destroy_codec(codec);
            return false;
        }

        int w = image->comps[0].w;
        int h = image->comps[0].h;
        int comps = image->numcomps;

        out.info.width = w;
        out.info.height = h;
        out.info.channels = comps;
        out.info.bit_depth = image->comps[0].prec;
        out.info.format = FORMAT::JP2K;
        out.info.valid = true;

        out.pixels.resize((size_t)w * h * comps);

        for (int i = 0; i < w * h; ++i) {
            for (int c = 0; c < comps; ++c) {
                int val = image->comps[c].data[i];

                // Normalize to 8-bit
                if (image->comps[c].prec > 8)
                    val >>= (image->comps[c].prec - 8);

                out.pixels[i * comps + c] = (uint8_t)val;
            }
        }

        opj_image_destroy(image);
        opj_stream_destroy(opj_stream);
        opj_destroy_codec(codec);

        return true;
    }

}  // namespace OpenEFT