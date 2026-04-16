// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once

#include <NMTK/NMTK.h>
#include <tinyfiledialogs.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OpenEFT {

    struct FileDialogOptions {
        std::string title = "Select File";
        std::string default_path = "./";

        std::vector<std::string> patterns;  // {"*.an2", "*.eft"}
        std::string filter_description = "Files";

        bool allow_multi_select = true;
    };
    class AsyncOpenFileDialog {
      public:
        using Callback = std::function<void(std::vector<std::string>)>;

        void Open(const FileDialogOptions& opts, Callback cb) {
            if (m_Open.exchange(true))
                return;

            std::thread([this, opts, cb]() {
                // Convert patterns → const char*
                std::vector<const char*> c_patterns;
                c_patterns.reserve(opts.patterns.size());

                for (auto& p : opts.patterns) {
                    c_patterns.push_back(p.c_str());
                }

                char* files = tinyfd_openFileDialog(
                    opts.title.c_str(), opts.default_path.c_str(),
                    static_cast<int>(c_patterns.size()),
                    c_patterns.empty() ? nullptr : c_patterns.data(),
                    opts.filter_description.c_str(),
                    opts.allow_multi_select ? 1 : 0);

                std::vector<std::string> results;

                if (files) {
                    char* file = std::strtok(files, "|");
                    while (file) {
                        results.emplace_back(file);
                        file = std::strtok(nullptr, "|");
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_Result = std::move(results);
                    m_Callback = cb;
                    m_Ready = true;
                }

                m_Open = false;
            }).detach();
        }

        void Pump() {
            if (!m_Ready)
                return;

            Callback cb;
            std::vector<std::string> result;

            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                cb = std::move(m_Callback);
                result = std::move(m_Result);
                m_Ready = false;
            }

            if (cb) {
                cb(std::move(result));
            }
        }

      private:
        std::atomic<bool> m_Open = false;
        std::atomic<bool> m_Ready = false;

        std::mutex m_Mutex;
        std::vector<std::string> m_Result;
        Callback m_Callback;
    };
}  // namespace OpenEFT
