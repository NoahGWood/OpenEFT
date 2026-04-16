// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#include <NMTK/NMTK.h>
#include <OpenEFT/OpenEFT.h>
#include <memory>

using namespace OpenEFT;

struct OpenEFTApp final : NMTK::IApp {
    void Register(std::shared_ptr<NMTK::AppContext> ctx) override {
        ctx->SetProject(ctx, std::move(std::make_unique<Project>(ctx)));
        ctx->window().AddMenu<FileMenu>(ctx);
        ctx->window().AddMenu<ProfileMenu>(ctx);
        ctx->window().AddMenu<SettingsMenu>(ctx);
        ctx->window().AddMenu<ValidateMenu>(ctx);
        
        ctx->window().AddPanel<FileExplorer>(ctx);
        ctx->window().AddPanel<FileHierarchyPanel>(ctx);
        ctx->window().AddPanel<RuleEditorPanel>(ctx);
        ctx->window().AddPanel<InspectorPanel>(ctx);
        ctx->window().AddPanel<ValidationPanel>(ctx);
    }
    void OnFrame(std::shared_ptr<NMTK::AppContext> ctx) override {}
};

int main(int argc, char** argv) {
    NMTK::AppDesc desc{.name = "OpenEFT", .args = {argc, argv}};

    return NMTK::RunApp(desc, [] { return std::make_unique<OpenEFTApp>(); });
}