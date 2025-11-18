#include "services/PoiService.h"

#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

#include <iostream>

namespace services
{
    auto PoiService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        repo_         = std::make_unique<repositories::PoiRepository>(dbClient);
        LOG_INFO << "[POI SERVICE] Plugin started";
    }

    auto PoiService::shutdown() -> void
    {
        repo_.reset();
        LOG_INFO << "[POI SERVICE] Plugin stopped";
    }

    auto
    PoiService::createPois(std::vector<dto::CreatePoiDto>&& dtos) -> drogon::Task<std::vector<int>>
    {
        for (auto& dto : dtos)
        {
            if (!dto.typeName.empty())
            {
                dto.typeId = findPoiTypeId(dto.typeName);
            }
        }
        // create via repository
        co_return co_await repo_->createManyPoiReturningId(dtos);
    }

    int PoiService::findPoiTypeId(const std::string& typeName)
    {
        if (typeName.empty())
            return 0;

        try
        {
            {
                std::lock_guard lock(cacheMutex_);
                auto            it = typeCache_.find(typeName);
                if (it != typeCache_.end())
                    return it->second;
            }

            auto dbClient = drogon::app().getDbClient();
            if (!dbClient)
            {
                LOG_ERROR << "[POI SERVICE] DB client is null";
                return 0;
            }

            int typeId = 0;

            auto sel = dbClient->execSqlSync("SELECT id FROM poi_types WHERE name=$1", typeName);

            if (!sel.empty())
            {
                typeId = sel[0]["id"].as<int>();
            }
            else
            {
                auto ins = dbClient->execSqlSync(
                    "INSERT INTO poi_types(name) VALUES($1) RETURNING id", typeName);

                if (!ins.empty())
                    typeId = ins[0]["id"].as<int>();
            }

            if (typeId != 0)
            {
                std::lock_guard lock(cacheMutex_);

                auto it = typeCache_.find(typeName);
                if (it != typeCache_.end())
                    return it->second;

                typeCache_.emplace(typeName, typeId);
            }

            return typeId;
        }
        catch (const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "[POI SERVICE] findPoiTypeIdSync error: " << e.base().what();
            return 0;
        }
    }

    auto PoiService::getPoiById(int id) -> drogon::Task<std::optional<repositories::models::Poi>>
    {
        co_return co_await repo_->getPoiById(id);
    }

    auto
    PoiService::getPoiByType(int typeId) -> drogon::Task<std::vector<repositories::models::Poi>>
    {
        co_return co_await repo_->getPoiByType(typeId);
    }

}  // namespace services
