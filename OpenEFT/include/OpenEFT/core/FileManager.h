#pragma once
#include <NMTK/NMTK.h>
#include <nnist/nnist.h>

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace OpenEFT {
    struct Selection{
        nnist::Record* record = nullptr;
        nnist::Field* field = nullptr;
        int recordId = -1;
        int fieldIdx = -1;
    };

    class FileManager {
      public:
        FileManager(std::shared_ptr<NMTK::AppContext> ctx)
            : m_AppContext(std::move(ctx)) {}

        void Open(const std::filesystem::path& path);
        void Close(const std::filesystem::path& path);
        void CloseAll();

        void SaveActive();
        void SaveActiveAs(const std::filesystem::path& path);
        void SaveAll();

        nnist::File& GetActive() { return m_Files[m_Active]; }
        std::filesystem::path& GetActivePath() { return m_Active; }

        std::unordered_map<std::filesystem::path, nnist::File>& All() {
            return m_Files;
        }
        bool HasActive() const {
            return !m_Active.empty() && m_Files.find(m_Active) != m_Files.end();
        }
        bool IsActive(const std::filesystem::path& path) {
            return m_Active == path;
        }
        void SetActive(const std::filesystem::path& path);

        void SetFiles(const std::unordered_map<std::filesystem::path,
                                               nnist::File>& files) {
            m_Files = files;
        }
        void SetSelection(const Selection& selection) { m_Selection = selection; }
        Selection& GetSelection() { return m_Selection; }

      private:
        std::filesystem::path m_Active;
        Selection m_Selection;
        std::unordered_map<std::filesystem::path, nnist::File> m_Files;
        std::shared_ptr<NMTK::AppContext> m_AppContext;
    };

}  // namespace OpenEFT
