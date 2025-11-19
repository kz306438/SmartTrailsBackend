#pragma once
#include <drogon/plugins/Plugin.h>

#include "repositories/PoiTypeRepository.h"

namespace services
{

    class PoiTypeService : public drogon::Plugin<PoiTypeService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

      public:
        [[nodiscard]] auto
        getPoiTypes() -> drogon::Task<std::vector<repositories::models::PoiTypes>>;

      private:
        std::unique_ptr<repositories::PoiTypeRepository> repo_;
    };

}  // namespace services
