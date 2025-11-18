#pragma once
#include <drogon/plugins/Plugin.h>

#include <memory>
#include <mutex>
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
        createPois(std::vector<dto::CreatePoiDto>&& dtos) -> drogon::Task<std::vector<int>>;

        [[nodiscard]] auto
        getPoiById(int id) -> drogon::Task<std::optional<repositories::models::Poi>>;

        [[nodiscard]] auto
        getPoiByType(int typeId) -> drogon::Task<std::vector<repositories::models::Poi>>;

      private:
        auto findPoiTypeId(const std::string& typeName) -> int;

      private:
        std::unique_ptr<repositories::PoiRepository> repo_;
        std::unordered_map<std::string, int>         typeCache_;
        std::mutex                                   cacheMutex_;
    };

}  // namespace services
