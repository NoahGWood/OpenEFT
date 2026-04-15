#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace OpenEFT {
    struct EnumDefinition {
        std::string name;
        std::vector<std::string> values;
    };

    class EnumRegistry {
      public:
        void Register(const std::string& name,
                      std::vector<std::string> values) {
            m_Enums[name] = std::move(values);
        }

        const std::vector<std::string>* Get(const std::string& name) const {
            auto it = m_Enums.find(name);
            if (it == m_Enums.end())
                return nullptr;
            return &it->second;
        }
        void EraseAt(const std::string& name, const int idx){
            if(idx < m_Enums[name].size()){
                m_Enums[name].erase(m_Enums[name].begin() + idx);
            }
        }
        void AddValue(const std::string& name, const std::string& value) {
            // if (!std::regex_match(v, std::regex("^[A-Z0-9_]+$"))) {
            //     CORE_WARN("Invalid enum format");
            //     return;
            // }
            m_Enums[name].push_back(value);
        }
        void Clear(){
            m_Enums.clear();
        }
        const auto All() const { return m_Enums; }

      private:
        std::unordered_map<std::string, std::vector<std::string>> m_Enums;
    };

    inline void to_json(nlohmann::json& j, const EnumRegistry& r) {
        j = r.All();
    }
    inline void from_json(const nlohmann::json& j, EnumRegistry& r) {
        r.Clear();
        for (auto& [name, values] : j.items()) {
            r.Register(name, values.get<std::vector<std::string>>());
        }
    }
}  // namespace OpenEFT
