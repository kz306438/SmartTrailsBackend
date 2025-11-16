#pragma once

#include <drogon/orm/CoroMapper.h>
#include <drogon/orm/DbClient.h>

#include <optional>

#include "models/UserPreferences.h"

namespace repositories
{
    namespace models = drogon_model::smarttrails;

    class UserPreferencesRepository
    {
      public:
        explicit UserPreferencesRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto createPreferences(int userId, double distanceKm, const std::string& poiTypeIds)
            -> drogon::Task<std::optional<models::UserPreferences>>;

        [[nodiscard]] auto
        getPreferencesById(int id) -> drogon::Task<std::optional<models::UserPreferences>>;
        [[nodiscard]] auto
        getPreferencesByUserId(int userId) -> drogon::Task<std::vector<models::UserPreferences>>;

        auto updatePreferences(const models::UserPreferences& preferences) -> drogon::Task<bool>;
        auto deletePreferences(int id) -> drogon::Task<bool>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };
}  // namespace repositories
