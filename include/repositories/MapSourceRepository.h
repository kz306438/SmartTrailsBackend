#pragma once

#include <drogon/orm/CoroMapper.h>
#include <drogon/orm/DbClient.h>

#include <optional>

#include "models/MapSources.h"

namespace repositories
{
    namespace models = drogon_model::smarttrails;

    class MapSourceRepository
    {
      public:
        explicit MapSourceRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto
        createMapSource(const std::string& name, const std::string& path,
                        bool isActive = false) -> drogon::Task<std::optional<models::MapSources>>;

        [[nodiscard]] auto
        getMapSourceById(int id) -> drogon::Task<std::optional<models::MapSources>>;
        [[nodiscard]] auto getActiveMapSource() -> drogon::Task<std::optional<models::MapSources>>;
        [[nodiscard]] auto getAllMapSources() -> drogon::Task<std::vector<models::MapSources>>;

        auto updateMapSource(const models::MapSources& mapSource) -> drogon::Task<bool>;
        auto deleteMapSource(int id) -> drogon::Task<bool>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };
}  // namespace repositories
