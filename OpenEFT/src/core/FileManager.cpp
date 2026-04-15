#include <OpenEFT/core/FileManager.h>

namespace OpenEFT {

    void FileManager::Open(const std::filesystem::path& path) {
        CORE_INFO("Loading File: {}", path.string());
        if(m_Files.find(path) != m_Files.end()){
            CLIENT_WARN("File already exists.");
            // For now return, later we'll add a popup to check
            // if the user wants to reload/reset the file
            return;
        }
        try {
            auto data = nnist::READ_FILE(path);
            auto file = nnist::PARSE_FILE(data);
            m_Files[path] = file;
        } catch(const std::runtime_error& e){
            CLIENT_ERROR("Error while parsing file {}: {}", path.string(), e.what());
        } catch(...){
            CLIENT_ERROR("Unknown exception.");
        }
        
    }
    void FileManager::Close(const std::filesystem::path& path) {
        if(m_Files.find(path) != m_Files.end()){
            m_Files.erase(path);
            return;
        }
        CLIENT_ERROR("File does not exist. Nothing to close.");
    }
    void FileManager::CloseAll() {
        m_Files.clear();
    }
    void FileManager::SetActive(const std::filesystem::path& path) {
        if(m_Files.find(path) == m_Files.end()){
            CLIENT_ERROR("File does not exist. Nothing to open");
            return;
        } else {
            m_Active = path;
        }
    }

    void FileManager::SaveActive() {
        if(std::filesystem::is_empty(m_Active)){
            CORE_WARN("No active file");
            return;
        }
        auto& file = GetActive();
        auto data = nnist::SERIALIZE_FILE(file);
        nnist::WRITE_FILE(m_Active, data);
        
    }

    void FileManager::SaveActiveAs(const std::filesystem::path& path){
        if(std::filesystem::is_empty(m_Active)){
            CORE_WARN("No active file");
            return;
        }
        auto& file = GetActive();
        auto data = nnist::SERIALIZE_FILE(file);
        nnist::WRITE_FILE(path, data);
    }

    void FileManager::SaveAll() {
        for(auto& [path, file] : m_Files) {
            auto data = nnist::SERIALIZE_FILE(file);
            nnist::WRITE_FILE(path, data);
        }
    }
}  // namespace OpenEFT