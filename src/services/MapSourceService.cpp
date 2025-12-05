#include "services/MapSourceService.h"

#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

#include <filesystem>
#include <fstream>

namespace services
{
    auto MapSourceService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        repo_         = std::make_unique<repositories::MapSourceRepository>(dbClient);
        LOG_INFO << "[MAP-SOURCE SERVICE] Plugin started";
    }

    auto MapSourceService::shutdown() -> void
    {
        repo_.reset();
        LOG_INFO << "[MAP-SOURCE SERVICE] Plugin stopped";
    }

    auto MapSourceService::createMapSource(const std::string& name, const std::string& path,
                                           bool isActive)
        -> drogon::Task<std::optional<repositories::models::MapSources>>
    {
        co_return co_await repo_->createMapSource(name, path, isActive);
    }

    auto MapSourceService::getMapSourceById(int id)
        -> drogon::Task<std::optional<repositories::models::MapSources>>
    {
        co_return co_await repo_->getMapSourceById(id);
    }

    auto MapSourceService::getAllMapSources()
        -> drogon::Task<std::vector<repositories::models::MapSources>>
    {
        co_return co_await repo_->getAllMapSources();
    }

    auto MapSourceService::updateMapSource(const repositories::models::MapSources& mapSource,
                                           const std::string& path) -> drogon::Task<bool>
    {
        auto activeMapSourceOpt = co_await repo_->getActiveMapSource();
        if (!activeMapSourceOpt)
            co_return false;

        int activeMapSourceId = *activeMapSourceOpt->getId();
        int mapSourceId       = *mapSource.getId();

        bool activeChanged = (activeMapSourceId != mapSourceId);

        auto mapSourceOpt = co_await repo_->updateMapSource(mapSource);
        if (!mapSourceOpt)
            co_return false;

        if (activeChanged && *mapSource.getIsActive())
        {
            if (path.empty())
            {
                LOG_ERROR << "[MAP_SOURCE SERVICE] Path is missing for active map source";
                co_return false;
            }

            const std::string triggerFile = "/app/maps/update.trigger";

            try
            {
                std::ofstream ofs(triggerFile, std::ios::trunc);
                if (!ofs.is_open())
                {
                    LOG_ERROR << "[MAP_SOURCE SERVICE] Cannot open update.trigger for writing";
                    co_return false;
                }

                ofs << path;
                ofs.close();
            }
            catch (const std::exception& e)
            {
                LOG_ERROR << "[MAP_SOURCE SERVICE] Failed to create update.trigger: " << e.what();
                co_return false;
            }
        }

        co_return true;
    }

    auto MapSourceService::deleteMapSource(int id) -> drogon::Task<bool>
    {
        co_return co_await repo_->deleteMapSource(id);
    }

    auto MapSourceService::getActiveMapSource()
        -> drogon::Task<std::optional<repositories::models::MapSources>>
    {
        co_return co_await repo_->getActiveMapSource();
    }

}  // namespace services
