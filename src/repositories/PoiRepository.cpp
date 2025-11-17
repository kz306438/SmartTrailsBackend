#include "repositories/PoiRepository.h"

#include <format>

using namespace drogon::orm;

namespace repositories
{

    static std::string escapeSql(const std::string& s)
    {
        std::string res;
        for (char c : s)
        {
            if (c == '\'')
                res += "''";
            else
                res += c;
        }
        return res;
    }

    PoiRepository::PoiRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

    auto PoiRepository::createManyPoiReturningId(const std::vector<dto::CreatePoiDto>& pois)
        -> drogon::Task<std::vector<int>>
    {
        try
        {
            if (pois.empty())
                co_return {};

            // Sql query preparing
            std::string sql = "INSERT INTO poi (name, city, type_id, coordinates, description, "
                              "map_source_id) VALUES ";

            for (size_t i = 0; i < pois.size(); ++i)
            {
                const auto& p = pois[i];
                sql +=
                    std::format("('{}','{}',{},{},'{}',{})", escapeSql(p.name.value_or("unknown")),
                                escapeSql(p.city.value_or("unknown")), p.typeId,
                                "ST_GeomFromText('" + p.coordinatesWkt() + "',4326)",
                                escapeSql(p.description.value_or("unknown")), p.mapSourceId);
                if (i + 1 < pois.size())
                    sql += ",";
            }
            sql += " RETURNING id;";

            // Query execution
            auto result = co_await dbClient_->execSqlCoro(sql);

            std::vector<int> ids;
            ids.reserve(result.size());

            for (auto const& row : result)
                ids.push_back(row["id"].as<int>());

            co_return ids;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (creatManyReturningIds)" << e.base().what();
            co_return {};
        }
    }

    auto PoiRepository::createPoi(const std::string& name, const std::string& city, int typeId,
                                  const std::string& coordinatesWkt, const std::string& description,
                                  int mapSourceId) -> drogon::Task<std::optional<models::Poi>>
    {
        try
        {
            const auto now = trantor::Date::now();

            static const std::string sql = R"(
            INSERT INTO poi
                (name, city, type_id, coordinates, description, map_source_id, created_at)
            VALUES
                ($1, $2, $3, ST_GeomFromText($4, 4326), $5, $6, $7)
            RETURNING
                id, name, city, type_id, ST_AsText(coordinates) AS coordinates,
                description, map_source_id, created_at
        )";

            auto result = co_await dbClient_->execSqlCoro(sql, name, city, typeId, coordinatesWkt,
                                                          description, mapSourceId, now);

            if (result.empty())
                co_return std::nullopt;

            const auto& row = result[0];
            models::Poi poi;
            poi.setId(row["id"].as<int>());
            poi.setName(row["name"].as<std::string>());
            poi.setCity(row["city"].as<std::string>());
            poi.setTypeId(row["type_id"].as<int>());
            poi.setCoordinates(row["coordinates"].as<std::string>());
            poi.setDescription(row["description"].as<std::string>());
            poi.setMapSourceId(row["map_source_id"].as<int>());
            poi.setCreatedAt(now);

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
