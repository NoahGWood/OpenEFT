#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/validator/TransactionValidator.h>

#include <vector>
#include <filesystem>

namespace OpenEFT {
    class ValidatorManager {
      public:
        ValidatorManager(std::shared_ptr<NMTK::AppContext> ctx)
            : m_AppContext(ctx) {}

        void ValidateActiveFile();

        void ExportValidationResults(const std::filesystem::path& path);

        const std::vector<Diagnostic>& GetDiagnostics() const {
            return m_Diagnostics;
        }
        void ClearDiagnostics() { m_Diagnostics.clear(); }
        void SetDiagnostics(const std::vector<Diagnostic>& dx) {
            m_Diagnostics = dx;
        }

      private:
        std::shared_ptr<NMTK::AppContext> m_AppContext;
        std::vector<Diagnostic> m_Diagnostics;
        TransactionValidator m_Validator;
    };
}  // namespace OpenEFT
