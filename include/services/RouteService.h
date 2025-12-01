#pragma once

#include <drogon/plugins/Plugin.h>

#include <optional>
#include <string>

#include "dto/RequestRouteDto.h"
#include "repositories/PoiRepository.h"
#include "repositories/RouteRepository.h"

namespace services
{

    class RouteService : public drogon::Plugin<RouteService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

      public:
        [[nodiscard]]
        auto createRoute(const dto::RequestRouteDto& dto)
            -> drogon::Task<std::optional<repositories::models::Routes>>;

        // [[nodiscard]]
        // auto getUserById(int userId) -> drogon::Task<std::optional<repositories::models::Users>>;

        // [[nodiscard]]
        // auto getUserByEmail(const std::string& email)
        //     -> drogon::Task<std::optional<repositories::models::Users>>;

        // [[nodiscard]]RouteRepository
        // auto getAllUsers() -> drogon::Task<std::vector<repositories::models::Users>>;

      private:
        std::unique_ptr<repositories::RouteRepository> routeRepo_;
        std::unique_ptr<repositories::PoiRepository>   poiRepo_;
    };

}  // namespace services
