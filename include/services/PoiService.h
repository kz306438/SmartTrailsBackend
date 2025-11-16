#pragma once
#include <drogon/plugins/Plugin.h>

#include <memory>
#include <vector>

#include "dto/CreatePoiDto.h"
#include "repositories/PoiRepository.h"

namespace services
{

    class PoiService : public drogon::Plugin<PoiService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

      public:
        [[nodiscard]] auto
        createPois(const std::vector<dto::CreatePoiDto>& dtos) -> drogon::Task<std::vector<int>>;

      private:
        std::unique_ptr<repositories::PoiRepository> repo_;
        std::unordered_map<std::string, int>         typeCache_;
    };

}  // namespace services
