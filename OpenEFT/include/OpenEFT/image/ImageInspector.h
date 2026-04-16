// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/image/Types.h>
#include <nnist/nnist.h>

namespace OpenEFT {
    class ImageInspector {
      public:
        virtual ~ImageInspector() = default;
        // Quick check (magic bytes, etc.)
        virtual bool CanHandle(const nnist::Record& record,
                               const nnist::Field& field,
                               std::shared_ptr<NMTK::AppContext> ctx) const = 0;
        // MUST be fast (header-only parsing)
        virtual ImageInfo Inspect(
            const nnist::Record& record, const nnist::Field& field,
            std::shared_ptr<NMTK::AppContext> ctx) const = 0;
        // optional (renderer only)
        virtual bool Decode(const nnist::Record& record,
                            const nnist::Field& field, Image& out,
                            std::shared_ptr<NMTK::AppContext> ctx) const {
            return false;
        }
        // optional (autofill)
        virtual SuggestionReport Suggest(const ImageInfo& info) const {
            return {};
        }
    };
}  // namespace OpenEFT
