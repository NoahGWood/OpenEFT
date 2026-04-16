// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/core/FileManager.h>
#include <OpenEFT/core/Setting.h>
#include <OpenEFT/image/Image.h>
#include <OpenEFT/profile/ProfileManager.h>
#include <OpenEFT/validator/ValidatorManager.h>

namespace OpenEFT {
    class Project : public NMTK::IProject {
      public:
        Project(std::shared_ptr<NMTK::AppContext> ctx) : m_AppContext(ctx) {
            m_FileManager = std::make_unique<FileManager>(ctx);
            m_Settings = std::make_unique<Setting>();
            m_ProfileManager = std::make_unique<ProfileManager>(ctx);
            m_ValidatorManager = std::make_unique<ValidatorManager>(ctx);
            m_ImageRegistry = std::make_unique<ImageRegistry>(ctx);
            m_ImageRegistry->Register<LegacyImageInspector>(); // Order matters!
            // m_ImageRegistry->Register<RawImageInspector>();
            m_ImageRegistry->Register<SignatureImageInspector>();
            // m_ImageRegistry->Register<Type14ImageInspector>();
            m_ImageRegistry->Register<TaggedImageInspector>();
        }
        FileManager& Files() { return *m_FileManager; }
        ProfileManager& Profiles() { return *m_ProfileManager; }
        Setting& Settings() { return *m_Settings; }
        ValidatorManager& Validator() { return *m_ValidatorManager; }
        ImageRegistry& Images() { return *m_ImageRegistry; }

      private:
        std::shared_ptr<NMTK::AppContext> m_AppContext;
        std::unique_ptr<FileManager> m_FileManager;
        std::unique_ptr<Setting> m_Settings;
        std::unique_ptr<ProfileManager> m_ProfileManager;
        std::unique_ptr<ValidatorManager> m_ValidatorManager;
        std::unique_ptr<ImageRegistry> m_ImageRegistry;
    };
}  // namespace OpenEFT
