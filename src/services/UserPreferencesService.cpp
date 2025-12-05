#include "services/UserPreferencesService.h"

#include <drogon/drogon.h>

namespace services
{
    auto UserPreferencesService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        repo_         = std::make_unique<repositories::UserPreferencesRepository>(dbClient);
        LOG_INFO << "[USER-PREFERENCES SERVICE] Plugin started";
    }

    auto UserPreferencesService::shutdown() -> void
    {
        repo_.reset();
        LOG_INFO << "[USER-PREFERENCES SERVICE] Plugin stopped";
    }

    auto UserPreferencesService::createPreferecnes(int userId, double distanceKm,
                                                   const std::vector<int>& poiTypeIds)
        -> drogon::Task<std::optional<repositories::models::UserPreferences>>
    {
        co_return co_await repo_->createPreferences(userId, distanceKm, poiTypeIds);
    }

}  // namespace services