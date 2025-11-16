#include "services/PoiService.h"

#include <drogon/drogon.h>

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

    auto PoiService::createPois(const std::vector<dto::CreatePoiDto>& dtos)
        -> drogon::Task<std::vector<int>>
    {
        std::vector<int> insertedIds;

        for (auto dto : dtos)
        {
            try
            {
                if (!dto.typeName.empty())
                {
                    const std::string tn = dto.typeName;
                    auto              it = typeCache_.find(tn);
                    if (it != typeCache_.end())
                    {
                        dto.typeId = it->second;
                    }
                    else
                    {
                        auto dbClient = drogon::app().getDbClient();
                        auto res      = co_await dbClient->execSqlCoro(
                            "SELECT id FROM poi_types WHERE name=$1", tn);
                        if (!res.empty())
                        {
                            int tid    = res[0]["id"].as<int>();
                            dto.typeId = tid;
                            typeCache_.emplace(tn, tid);
                        }
                        else
                        {
                            auto ins = co_await dbClient->execSqlCoro(
                                "INSERT INTO poi_types(name) VALUES($1) RETURNING id", tn);
                            if (!ins.empty())
                            {
                                int tid    = ins[0]["id"].as<int>();
                                dto.typeId = tid;
                                typeCache_.emplace(tn, tid);
                            }
                        }
                    }
                }

                // fields preparing for repository
                std::string name        = dto.name.value_or("unknown");
                std::string city        = dto.city.value_or("unknown");
                std::string description = dto.description.value_or("unknown");

                // create via repository
                auto optModel = co_await repo_->createPoi(
                    name, city, dto.typeId, dto.coordinatesWkt(), description, dto.mapSourceId);

                if (optModel && optModel->getId())
                {
                    insertedIds.push_back(*optModel->getId());
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR << "[POI SERVICE] failed to create POI '" << dto.name.value_or("unknown")
                          << "': " << e.what();
            }
        }

        co_return insertedIds;
    }

}  // namespace services
