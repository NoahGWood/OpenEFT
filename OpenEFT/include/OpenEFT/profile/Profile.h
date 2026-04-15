#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/rules/EnumRegistry.h>
#include <OpenEFT/rules/RuleSerDes.h>
#include <OpenEFT/rules/Rules.h>

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace OpenEFT {
    using json = nlohmann::json;
    class Profile {
      public:
        Profile(const std::filesystem::path& path) : m_File(path) {
            LoadFromFile();
        }
        Profile() {}

        auto& AllTransactions() { return m_Transactions; }
        TransactionRule& GetTransaction(const std::string& name) {
            if (m_Transactions.find(name) == m_Transactions.end()) {
                CORE_ERROR("Transaction with ID {} not found. Creating new.",
                           name);
                m_Transactions[name] = TransactionRule{name};
            }
            return m_Transactions[name];  // Get or Create
        }
        EnumRegistry& Enums() { return m_EnumRegistry; }
        
        const std::string& GetName() const { return m_Name; }
        const int GetVersion() const { return m_Version; }
        void SetName(const std::string& name) { m_Name = name; }
        const std::filesystem::path& GetPath() const { return m_File; }
        void SetPath(const std::filesystem::path& path){
            m_File = path;
        }

        void SaveToFile();

        void LoadFromFile();

      private:
        std::string m_Name;
        int m_Version = 1;
        EnumRegistry m_EnumRegistry;
        std::unordered_map<std::string, TransactionRule> m_Transactions;
        std::filesystem::path m_File;

        friend void to_json(json& j, const Profile& p);
        friend void from_json(const json& j, Profile& p);
    };

    inline void to_json(json& j, const Profile& p) {
        j = json{{"name", p.m_Name},
                 {"version", p.m_Version},
                 {"transactions", p.m_Transactions},
                {"enums", p.m_EnumRegistry}};
    }

    inline void from_json(const json& j, Profile& p) {
        p.m_Name = j.value("name", "UNNAMED PROFILE");
        p.m_Version = j.value("version", 0);

        if (j.contains("transactions")) {
            p.m_Transactions =
                j.at("transactions")
                    .get<std::unordered_map<std::string, TransactionRule>>();
        }
        if(j.contains("enums")){
            p.m_EnumRegistry = j.at("enums").get<EnumRegistry>();
        }
    }
}  // namespace OpenEFT
