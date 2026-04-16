// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/UI/widgets/ANSIFileUI.h>
#include <OpenEFT/cmd/Commands.h>
#include <nnist/nnist.h>

namespace OpenEFT {
    struct FileHierarchyPanel : NMTK::IPanel {
        FileHierarchyPanel(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IPanel(std::move(ctx)) {
            m_Name = "File Hierarchy";
        }

        void Render() override ;
    };
}  // namespace OpenEFT
