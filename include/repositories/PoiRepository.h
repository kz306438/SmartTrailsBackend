#pragma once

#include <drogon/orm/CoroMapper.h>
#include <drogon/orm/DbClient.h>

#include <optional>

#include "models/Poi.h"

namespace repositories
{
    namespace models = drogon_model::smarttrails;

    class PoiRepository
    {
      public:
        explicit PoiRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto createPoi(const std::string& name, const std::string& city, int typeId,
                       const std::string& coordinatesWkt, const std::string& description,
                       int mapSourceId) -> drogon::Task<std::optional<models::Poi>>;

        [[nodiscard]] auto getPoiById(int id) -> drogon::Task<std::optional<models::Poi>>;
        [[nodiscard]] auto getPoiByType(int typeId) -> drogon::Task<std::vector<models::Poi>>;
        [[nodiscard]] auto getAllPoi() -> drogon::Task<std::vector<models::Poi>>;

        auto updatePoi(const models::Poi& poi) -> drogon::Task<bool>;
        auto deletePoi(int id) -> drogon::Task<bool>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };
}  // namespace repositories
