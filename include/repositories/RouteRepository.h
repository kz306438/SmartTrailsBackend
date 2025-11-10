#pragma once

#include <drogon/orm/CoroMapper.h>
#include <drogon/orm/DbClient.h>

#include <optional>

#include "models/Routes.h"

namespace repositories
{
    namespace models = drogon_model::smarttrails;

    class RoutesRepository
    {
      public:
        explicit RoutesRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto createRoute(int userId, int preferenceId, const std::string& name,
                         const std::string& startPointWkt, const std::string& routeLineWkt,
                         double distanceKm,
                         int    mapSourceId) -> drogon::Task<std::optional<models::Routes>>;

        [[nodiscard]] auto getRouteById(int id) -> drogon::Task<std::optional<models::Routes>>;
        [[nodiscard]] auto
        getRoutesByUserId(int userId) -> drogon::Task<std::vector<models::Routes>>;
        [[nodiscard]] auto getAllRoutes() -> drogon::Task<std::vector<models::Routes>>;

        auto updateRoute(const models::Routes& route) -> drogon::Task<bool>;
        auto deleteRoute(int id) -> drogon::Task<bool>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };
}  // namespace repositories
