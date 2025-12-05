#pragma once

#include <drogon/orm/CoroMapper.h>
#include <drogon/orm/DbClient.h>

#include <vector>

#include "models/RoutePoi.h"

namespace repositories
{
    namespace models = drogon_model::smarttrails;

    class RoutePoiRepository
    {
      public:
        explicit RoutePoiRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto addPoiToRoute(int routeId, int poiId) -> drogon::Task<bool>;
        auto removePoiFromRoute(int routeId, int poiId) -> drogon::Task<bool>;
        [[nodiscard]] auto
        getPoisByRoute(int routeId) -> drogon::Task<std::vector<models::RoutePoi>>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };
}  // namespace repositories
