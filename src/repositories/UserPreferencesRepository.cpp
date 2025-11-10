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
            models::UserPreferences prefs;
            prefs.setUserId(userId);
            // prefs.setPoiTypeIds(poiTypeIds);  // предполагается, что поле типа std::vector<int>
            prefs.setUpdatedAt(trantor::Date::now());

            CoroMapper<models::UserPreferences> mapper(dbClient_);
            co_await mapper.insert(prefs);

            co_return prefs;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createPreferences): " << e.base().what();
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
