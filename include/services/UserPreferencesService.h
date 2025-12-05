#pragma once

#include <drogon/plugins/Plugin.h>

#include <optional>
#include <string>

#include "repositories/UserPreferencesRepository.h"

namespace services
{

    class UserPreferencesService : public drogon::Plugin<UserPreferencesService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

      public:
        auto createPreferecnes(int userId, double distanceKm, const std::vector<int>& poiTypeIds)
            -> drogon::Task<std::optional<repositories::models::UserPreferences>>;

      private:
        std::unique_ptr<repositories::UserPreferencesRepository> repo_;
    };

}  // namespace services
