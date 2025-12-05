#pragma once
#include <drogon/plugins/Plugin.h>

#include "repositories/MapSourceRepository.h"

namespace services
{

    class MapSourceService : public drogon::Plugin<MapSourceService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

      public:
        auto createMapSource(const std::string& name, const std::string& path,
                             bool isActive = false)
            -> drogon::Task<std::optional<repositories::models::MapSources>>;

        [[nodiscard]] auto
        getMapSourceById(int id) -> drogon::Task<std::optional<repositories::models::MapSources>>;

        [[nodiscard]] auto
        getAllMapSources() -> drogon::Task<std::vector<repositories::models::MapSources>>;

        [[nodiscard]] auto
        getActiveMapSource() -> drogon::Task<std::optional<repositories::models::MapSources>>;

        auto updateMapSource(const repositories::models::MapSources& mapSource,
                             const std::string&                      path) -> drogon::Task<bool>;
        auto deleteMapSource(int id) -> drogon::Task<bool>;

      private:
        std::unique_ptr<repositories::MapSourceRepository> repo_;
    };

}  // namespace services
