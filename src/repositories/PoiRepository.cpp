#include "repositories/PoiRepository.h"

using namespace drogon::orm;

namespace repositories
{

    PoiRepository::PoiRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

    auto PoiRepository::createPoi(const std::string& name, const std::string& city, int typeId,
                                  const std::string& coordinatesWkt, const std::string& description,
                                  int mapSourceId) -> drogon::Task<std::optional<models::Poi>>
    {
        try
        {
            models::Poi poi;
            poi.setName(name);
            poi.setCity(city);
            poi.setTypeId(typeId);
            poi.setDescription(description);
            poi.setMapSourceId(mapSourceId);
            poi.setCreatedAt(trantor::Date::now());

            // Преобразуем WKT в PostGIS geometry через SQL
            auto sql    = "SELECT ST_GeomFromText($1, 4326)";
            auto result = co_await dbClient_->execSqlCoro(sql, coordinatesWkt);
            if (!result.empty())
            {
                auto geom = result[0][0].as<std::string>();
                poi.setCoordinates(geom);
            }

            CoroMapper<models::Poi> mapper(dbClient_);
            co_await mapper.insert(poi);

            co_return poi;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createPoi): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto PoiRepository::getPoiById(int id) -> drogon::Task<std::optional<models::Poi>>
    {
        try
        {
            CoroMapper<models::Poi> mapper(dbClient_);
            auto                    poi = co_await mapper.findByPrimaryKey(id);
            co_return poi;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getPoiById): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto PoiRepository::getPoiByType(int typeId) -> drogon::Task<std::vector<models::Poi>>
    {
        try
        {
            CoroMapper<models::Poi> mapper(dbClient_);
            auto                    pois = co_await mapper.findBy(
                Criteria(models::Poi::Cols::_type_id, CompareOperator::EQ, typeId));
            co_return pois;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getPoiByType): " << e.base().what();
            co_return {};
        }
    }

    auto PoiRepository::getAllPoi() -> drogon::Task<std::vector<models::Poi>>
    {
        try
        {
            CoroMapper<models::Poi> mapper(dbClient_);
            auto                    pois = co_await mapper.findAll();
            co_return pois;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (getAllPoi): " << e.base().what();
            co_return {};
        }
    }

    auto PoiRepository::updatePoi(const models::Poi& poi) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::Poi> mapper(dbClient_);
            co_await mapper.update(poi);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (updatePoi): " << e.base().what();
            co_return false;
        }
    }

    auto PoiRepository::deletePoi(int id) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::Poi> mapper(dbClient_);
            co_await mapper.deleteByPrimaryKey(id);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (deletePoi): " << e.base().what();
            co_return false;
        }
    }

}  // namespace repositories
