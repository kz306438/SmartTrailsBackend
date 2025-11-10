#include "repositories/MapSourceRepository.h"

using namespace drogon::orm;

namespace repositories
{

    MapSourceRepository::MapSourceRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient))
    {
    }

    auto MapSourceRepository::createMapSource(const std::string& name, const std::string& path,
                                              bool isActive)
        -> drogon::Task<std::optional<models::MapSources>>
    {
        try
        {
            models::MapSources mapSource;
            mapSource.setName(name);
            mapSource.setPath(path);
            mapSource.setIsActive(isActive);
            mapSource.setCreatedAt(trantor::Date::now());

            CoroMapper<models::MapSources> mapper(dbClient_);
            co_await mapper.insert(mapSource);

            co_return mapSource;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createMapSource): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto
    MapSourceRepository::getMapSourceById(int id) -> drogon::Task<std::optional<models::MapSources>>
    {
        try
        {
            CoroMapper<models::MapSources> mapper(dbClient_);
            auto                           mapSource = co_await mapper.findByPrimaryKey(id);
            co_return mapSource;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getMapSourceById): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto
    MapSourceRepository::getActiveMapSource() -> drogon::Task<std::optional<models::MapSources>>
    {
        try
        {
            CoroMapper<models::MapSources> mapper(dbClient_);
            auto                           active = co_await mapper.findOne(
                Criteria(models::MapSources::Cols::_is_active, CompareOperator::EQ, true));
            co_return active;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getActiveMapSource): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto MapSourceRepository::getAllMapSources() -> drogon::Task<std::vector<models::MapSources>>
    {
        try
        {
            CoroMapper<models::MapSources> mapper(dbClient_);
            auto                           sources = co_await mapper.findAll();
            co_return sources;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (getAllMapSources): " << e.base().what();
            co_return {};
        }
    }

    auto
    MapSourceRepository::updateMapSource(const models::MapSources& mapSource) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::MapSources> mapper(dbClient_);
            co_await mapper.update(mapSource);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (updateMapSource): " << e.base().what();
            co_return false;
        }
    }

    auto MapSourceRepository::deleteMapSource(int id) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::MapSources> mapper(dbClient_);
            co_await mapper.deleteByPrimaryKey(id);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (deleteMapSource): " << e.base().what();
            co_return false;
        }
    }

}  // namespace repositories
