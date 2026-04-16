#include <OpenEFT/OpenEFT.h>
#include <OpenEFT/rules/RuleMacros.h>
#include <OpenEFT/rules/Rules.h>
#include <OpenEFT/rules/RuleSerDes.h>
#include <OpenEFT/validator/AN2DocumentView.h>
#include <OpenEFT/validator/Helpers.h>
#include <OpenEFT/validator/ValidatorManager.h>

#include <fstream>
#include <iomanip>

namespace OpenEFT {
    void ValidatorManager::ValidateActiveFile() {
        auto& proj = m_AppContext->project();
        auto& project = static_cast<Project&>(proj);
        auto& pmgr = project.Profiles();
        auto& fmgr = project.Files();
        // Get Active Profile
        auto* profile = pmgr.GetActive();
        if (!profile) {
            CORE_ERROR("No active profile set.");
            return;
        }
        // Make sure a tx is active
        auto tx_name = pmgr.GetActiveTransactionName();
        if (tx_name.empty()) {
            CORE_ERROR("No active transaction selected.");
            return;
        }
        // Get Active File
        if (!fmgr.HasActive()) {
            CORE_ERROR("No active file set.");
            return;
        }
        auto& file = fmgr.GetActive();
        auto& tx = profile->GetTransaction(tx_name);
        // Create a new document view to walk file
        AN2DocumentView doc_view(file);
        // Run Validation
        m_Diagnostics = m_Validator.Validate(tx, doc_view);
    }

    void ValidatorManager::ExportValidationResults(
        const std::filesystem::path& path) {
        std::ofstream out(path);
        if (!out.is_open()) {
            CORE_ERROR("Failed to open export file.");
            return;
        }
        out << "Severity,Record,Field,Message"
            << "\n";
        for (auto& dx : m_Diagnostics) {
            out << SeverityText(dx.severity) << ",";
            out << "Type-" << dx.record_type << ",";
            out << dx.record_type << "." << std::setw(3) << std::setfill('0')
                << dx.field_id << ",";
            out << dx.message << "\n";
        }
        out.close();
        CORE_INFO("Validation results exported.");
    }
}  // namespace OpenEFT
