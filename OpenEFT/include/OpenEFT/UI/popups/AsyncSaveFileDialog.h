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
    struct SaveFileDialogOptions {
        std::string title = "Save File";
        std::string default_path = "output.eft";

        std::vector<std::string> patterns;  // {"*.eft"}
        std::string filter_description = "File";

        bool confirm_overwrite = true;  // tinyfd handles this internally
    };
    class AsyncSaveFileDialog {
      public:
        using Callback = std::function<void(std::string)>;

        void Open(const SaveFileDialogOptions& opts, Callback cb) {
            if (m_Open.exchange(true))
                return;

            std::thread([this, opts, cb]() {
                // Convert patterns → const char*
                std::vector<const char*> c_patterns;
                c_patterns.reserve(opts.patterns.size());

                for (auto& p : opts.patterns) {
                    c_patterns.push_back(p.c_str());
                }

                char* file = tinyfd_saveFileDialog(
                    opts.title.c_str(), opts.default_path.c_str(),
                    static_cast<int>(c_patterns.size()),
                    c_patterns.empty() ? nullptr : c_patterns.data(),
                    opts.filter_description.c_str());

                std::string result;
                if (file) {
                    result = file;
                }

                {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_Result = std::move(result);
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
            std::string result;

            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                cb = std::move(m_Callback);
                result = std::move(m_Result);
                m_Ready = false;
            }

            if (cb && !result.empty()) {
                cb(std::move(result));
            }
        }

        bool IsOpen() const { return m_Open; }

      private:
        std::atomic<bool> m_Open = false;
        std::atomic<bool> m_Ready = false;

        std::mutex m_Mutex;
        std::string m_Result;
        Callback m_Callback;
    };
}  // namespace OpenEFT
