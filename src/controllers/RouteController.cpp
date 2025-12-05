#include "controllers/RouteController.h"

#include <drogon/orm/Exception.h>

#include "services/RouteService.h"
#include "utils/JsonResponseUtil.h"

namespace controllers
{

    static auto parseGeoJsonString(const std::string& s) -> Json::Value
    {
        if (s.empty())
            return Json::nullValue;

        Json::Value             out;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string                       errs;

        if (reader->parse(s.c_str(), s.c_str() + s.size(), &out, &errs))
        {
            return out;
        }
        return Json::nullValue;
    }

    auto RouteController::getAll(drogon::HttpRequestPtr req,
                                 int user_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto db = drogon::app().getDbClient();
        try
        {
            constexpr const char* sql =
                "SELECT id, name, distance_km FROM routes WHERE user_id = $1 ORDER BY id DESC";

            auto routesResult = co_await db->execSqlCoro(sql, user_id);

            Json::Value jsonRoutes(Json::arrayValue);

            for (const auto& row : routesResult)
            {
                Json::Value routeJson;
                int         routeId = row["id"].as<int>();

                routeJson["id"]   = routeId;
                routeJson["name"] = row["name"].as<std::string>();

                // Generate a block of preferences
                Json::Value prefs;
                prefs["distance_km"] = row["distance_km"].as<double>();

                // Get briefly information about route
                constexpr const char* poiSql = "SELECT p.id, p.name, p.type_id "
                                               "FROM poi p "
                                               "JOIN route_poi rp ON p.id = rp.poi_id "
                                               "WHERE rp.route_id = $1";

                auto poiResult = co_await db->execSqlCoro(poiSql, routeId);

                Json::Value poiBriefArray(Json::arrayValue);
                for (const auto& poiRow : poiResult)
                {
                    Json::Value p;
                    p["id"]      = poiRow["id"].as<int>();
                    p["name"]    = poiRow["name"].as<std::string>();
                    p["type_id"] = poiRow["type_id"].as<int>();
                    poiBriefArray.append(p);
                }

                routeJson["pois_on_route"] = poiBriefArray;
                routeJson["preferences"]   = prefs;

                jsonRoutes.append(routeJson);
            }

            co_return drogon::HttpResponse::newHttpJsonResponse(jsonRoutes);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "Error in getAll routes: " << e.what();
            co_return utils::makeJsonError("Internal Server Error",
                                           drogon::k500InternalServerError);
        }
    }

    auto RouteController::getOne(drogon::HttpRequestPtr req, int user_id,
                                 int route_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto routeJsonOpt = co_await fetchRouteWithGeoJson(route_id);
            if (!routeJsonOpt)
            {
                co_return utils::makeJsonError("Route not found", drogon::k404NotFound);
            }
            co_return drogon::HttpResponse::newHttpJsonResponse(*routeJsonOpt);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "Error fetching route: " << e.what();
            co_return utils::makeJsonError("Internal Error", drogon::k500InternalServerError);
        }
    }

    auto RouteController::generate(drogon::HttpRequestPtr req,
                                   int user_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            // Validation
            auto jsonBody = req->getJsonObject();
            if (!jsonBody)
                co_return utils::makeJsonError("Body cannot be empty!", drogon::k400BadRequest);

            auto [dtoOpt, errorMsg] = parseAndValidateGenerateRequest(jsonBody, user_id);
            if (!dtoOpt)
                co_return utils::makeJsonError(errorMsg, drogon::k400BadRequest);

            const auto& dto = *dtoOpt;

            // Service call
            auto service    = drogon::app().getPlugin<services::RouteService>();
            auto maybeRoute = co_await service->createRoute(dto);

            if (!maybeRoute)
                co_return utils::makeJsonError("Failed to create route",
                                               drogon::k500InternalServerError);

            // Response generation
            auto responseJsonOpt = co_await fetchRouteWithGeoJson(*maybeRoute.value().getId());

            if (!responseJsonOpt)
                co_return utils::makeJsonError("Route created but failed to retrieve details",
                                               drogon::k500InternalServerError);

            co_return drogon::HttpResponse::newHttpJsonResponse(*responseJsonOpt);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[ROUTE CONTROLLER] Error (generate): " << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto RouteController::updateOne(drogon::HttpRequestPtr req, int user_id,
                                    int route_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto db = drogon::app().getDbClient();
        try
        {
            auto jsonBody = req->getJsonObject();
            if (!jsonBody)
                co_return utils::makeJsonError("Body cannot be empty", drogon::k400BadRequest);

            if (!jsonBody->isMember("name"))
                co_return utils::makeJsonError("Field 'name' is required for update",
                                               drogon::k400BadRequest);

            std::string newName = (*jsonBody)["name"].asString();
            if (newName.empty())
                co_return utils::makeJsonError("Name cannot be empty", drogon::k400BadRequest);

            // Update only if route belong to user
            constexpr const char* sql =
                "UPDATE routes SET name = $1 WHERE id = $2 AND user_id = $3";

            auto result = co_await db->execSqlCoro(sql, newName, route_id, user_id);

            if (result.affectedRows() == 0)
            {
                // Or route not found, or don't belong to user
                co_return utils::makeJsonError("Route not found or access denied",
                                               drogon::k404NotFound);
            }

            Json::Value resp;
            resp["status"] = "updated";
            resp["id"]     = route_id;
            resp["name"]   = newName;

            co_return drogon::HttpResponse::newHttpJsonResponse(resp);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "Error in updateOne: " << e.what();
            co_return utils::makeJsonError("Internal Server Error",
                                           drogon::k500InternalServerError);
        }
    }

    auto RouteController::deleteOne(drogon::HttpRequestPtr req, int user_id,
                                    int route_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto db = drogon::app().getDbClient();
        try
        {
            // Delete only if route belong to user
            constexpr const char* sql = "DELETE FROM routes WHERE id = $1 AND user_id = $2";

            auto result = co_await db->execSqlCoro(sql, route_id, user_id);

            if (result.affectedRows() == 0)
            {
                co_return utils::makeJsonError("Route not found or access denied",
                                               drogon::k404NotFound);
            }

            // Return 200 OK or 204 No Content
            Json::Value resp;
            resp["status"] = "deleted";
            resp["id"]     = route_id;
            co_return drogon::HttpResponse::newHttpJsonResponse(resp);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "Error in deleteOne: " << e.what();
            co_return utils::makeJsonError("Internal Server Error",
                                           drogon::k500InternalServerError);
        }
    }

    auto RouteController::parseAndValidateGenerateRequest(const std::shared_ptr<Json::Value>& json,
                                                          int user_id)
        -> std::pair<std::optional<dto::RequestRouteDto>, std::string>
    {
        if (user_id <= 0)
            return {std::nullopt, "Invalid user id"};

        const std::vector<std::string> requiredFields = {"name", "start_point", "distance",
                                                         "poi_types"};
        for (const auto& field : requiredFields)
        {
            if (!json->isMember(field))
                return {std::nullopt, "Missing '" + field + "' in request body"};
        }

        const auto& poiArr = (*json)["poi_types"];
        if (!poiArr.isArray())
            return {std::nullopt, "'poi_types' must be an array"};

        std::vector<int> poiTypes;
        poiTypes.reserve(poiArr.size());
        for (const auto& elem : poiArr)
        {
            if (!elem.isInt())
                return {std::nullopt, "'poi_types' must contain integers"};
            poiTypes.push_back(elem.asInt());
        }

        dto::RequestRouteDto dto;
        dto.user_id     = user_id;
        dto.name        = (*json)["name"].asString();
        dto.start_point = (*json)["start_point"].asString();
        dto.distance    = (*json)["distance"].asDouble();
        dto.poi_types   = std::move(poiTypes);

        return {dto, ""};
    }

    auto
    RouteController::fetchRouteWithGeoJson(int route_id) -> drogon::Task<std::optional<Json::Value>>
    {
        auto db = drogon::app().getDbClient();

        try
        {
            // Get general information about route
            constexpr const char* routeSql = "SELECT id, name, distance_km, "
                                             "ST_AsGeoJSON(start_point) AS start_point_json, "
                                             "ST_AsGeoJSON(route_line) AS route_line_json "
                                             "FROM routes WHERE id=$1";

            auto routeResult = co_await db->execSqlCoro(routeSql, route_id);

            if (routeResult.empty())
                co_return std::nullopt;

            const auto& r = routeResult[0];
            Json::Value resp;

            if (!r["id"].isNull())
                resp["id"] = r["id"].as<int>();
            if (!r["name"].isNull())
                resp["name"] = r["name"].as<std::string>();
            if (!r["distance_km"].isNull())
                resp["distance_km"] = r["distance_km"].as<double>();

            if (!r["start_point_json"].isNull())
                resp["start_point"] = parseGeoJsonString(r["start_point_json"].as<std::string>());

            if (!r["route_line_json"].isNull())
                resp["route_line"] = parseGeoJsonString(r["route_line_json"].as<std::string>());

            // Getting relating POI
            constexpr const char* poiSql = "SELECT p.id, p.name, p.description, p.type_id, "
                                           "ST_AsGeoJSON(p.coordinates) AS location_json "
                                           "FROM poi p "
                                           "JOIN route_poi rp ON p.id = rp.poi_id "
                                           "WHERE rp.route_id = $1";

            auto poiResult = co_await db->execSqlCoro(poiSql, route_id);

            Json::Value poisArray(Json::arrayValue);

            for (const auto& row : poiResult)
            {
                Json::Value poiObj;
                if (!row["id"].isNull())
                    poiObj["id"] = row["id"].as<int>();
                if (!row["name"].isNull())
                    poiObj["name"] = row["name"].as<std::string>();
                if (!row["description"].isNull())
                    poiObj["description"] = row["description"].as<std::string>();
                if (!row["type_id"].isNull())
                    poiObj["type_id"] = row["type_id"].as<int>();

                if (!row["location_json"].isNull())
                {
                    poiObj["location"] = parseGeoJsonString(row["location_json"].as<std::string>());
                }

                poisArray.append(poiObj);
            }

            resp["pois"] = poisArray;

            co_return resp;
        }
        catch (const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "DB Error in fetchRouteWithFullData: " << e.base().what();
            co_return std::nullopt;
        }
    }

}  // namespace controllers