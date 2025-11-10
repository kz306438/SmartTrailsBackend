#pragma once

#include <drogon/orm/CoroMapper.h>
#include <drogon/orm/DbClient.h>

#include <optional>

#include "models/PoiTypes.h"

namespace repositories
{
    namespace models = drogon_model::smarttrails;

    class PoiTypeRepository
    {
      public:
        explicit PoiTypeRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto
        createPoiType(const std::string& name) -> drogon::Task<std::optional<models::PoiTypes>>;
        [[nodiscard]] auto getPoiTypeById(int id) -> drogon::Task<std::optional<models::PoiTypes>>;
        [[nodiscard]] auto
        getPoiTypeByName(const std::string& name) -> drogon::Task<std::optional<models::PoiTypes>>;
        [[nodiscard]] auto getAllPoiTypes() -> drogon::Task<std::vector<models::PoiTypes>>;

        auto updatePoiType(const models::PoiTypes& poiType) -> drogon::Task<bool>;
        auto deletePoiType(int id) -> drogon::Task<bool>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };
}  // namespace repositories
