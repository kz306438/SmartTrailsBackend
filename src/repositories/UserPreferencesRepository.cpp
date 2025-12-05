#include "repositories/UserPreferencesRepository.h"

using namespace drogon::orm;

namespace repositories
{

    UserPreferencesRepository::UserPreferencesRepository(DbClientPtr dbClient)
        : dbClient_(std::move(dbClient))
    {
    }

    auto UserPreferencesRepository::createPreferences(int userId, double distanceKm,
                                                      const std::vector<int>& poiTypeIds)
        -> drogon::Task<std::optional<models::UserPreferences>>
    {
        try
        {
            std::string poiArray = "{";
            for (size_t i = 0; i < poiTypeIds.size(); ++i)
            {
                poiArray += std::to_string(poiTypeIds[i]);
                if (i + 1 < poiTypeIds.size())
                    poiArray += ",";
            }
            poiArray += "}";

            const char* sql =
                "INSERT INTO user_preferences (user_id, preferred_distance_km, poi_type_ids, "
                "updated_at) "
                "VALUES ($1, $2, $3, NOW()) "
                "RETURNING id, user_id, preferred_distance_km, poi_type_ids, updated_at";

            auto result = co_await dbClient_->execSqlCoro(sql, userId, distanceKm, poiArray);

            if (result.empty())
            {
                LOG_ERROR << "[REPOSITORY] createPreferences: empty RETURNING";
                co_return std::nullopt;
            }

            const auto&             row = result[0];
            models::UserPreferences prefs;

            prefs.setId(row["id"].as<int>());
            prefs.setUserId(row["user_id"].as<int>());
            prefs.setPreferredDistanceKm(row["preferred_distance_km"].as<std::string>());

            prefs.setPoiTypeIds(row["poi_type_ids"].as<std::string>());

            co_return prefs;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createPreferences SQL): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto UserPreferencesRepository::getPreferencesById(int id)
        -> drogon::Task<std::optional<models::UserPreferences>>
    {
        try
        {
            CoroMapper<models::UserPreferences> mapper(dbClient_);
            auto                                prefs = co_await mapper.findByPrimaryKey(id);
            co_return prefs;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getPreferencesById): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto UserPreferencesRepository::getPreferencesByUserId(int userId)
        -> drogon::Task<std::vector<models::UserPreferences>>
    {
        try
        {
            CoroMapper<models::UserPreferences> mapper(dbClient_);
            auto                                prefs = co_await mapper.findBy(
                Criteria(models::UserPreferences::Cols::_user_id, CompareOperator::EQ, userId));
            co_return prefs;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getPreferencesByUserId): " << e.base().what();
            co_return {};
        }
    }

    auto UserPreferencesRepository::updatePreferences(const models::UserPreferences& preferences)
        -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::UserPreferences> mapper(dbClient_);
            co_await mapper.update(preferences);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (updatePreferences): " << e.base().what();
            co_return false;
        }
    }

    auto UserPreferencesRepository::deletePreferences(int id) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::UserPreferences> mapper(dbClient_);
            co_await mapper.deleteByPrimaryKey(id);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (deletePreferences): " << e.base().what();
            co_return false;
        }
    }

}  // namespace repositories
