#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/image/ImageInspector.h>
#include <OpenEFT/image/Types.h>

#include <memory>

namespace OpenEFT {
    class ImageRegistry {
      public:
        ImageRegistry(std::shared_ptr<NMTK::AppContext> ctx)
            : m_AppContext(std::move(ctx)) {}

        template <typename T, typename... Args>
        void Register(Args&&... args) {
            m_Inspectors.push_back(
                std::make_unique<T>(std::forward<Args>(args)...));
        }

        const ImageInspector* Resolve(const nnist::Record& record,
                                      const nnist::Field& field) const {
            for (const auto& i : m_Inspectors) {
                if (i->CanHandle(record, field, m_AppContext)) {
                    return i.get();
                }
            }
            return nullptr;
        }

      private:
        std::shared_ptr<NMTK::AppContext> m_AppContext;
        std::vector<std::unique_ptr<ImageInspector>> m_Inspectors;
    };
}  // namespace OpenEFT
