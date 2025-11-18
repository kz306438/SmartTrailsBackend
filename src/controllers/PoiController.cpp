#include "controllers/PoiController.h"

#include <filesystem>
#include <fstream>

#include "services/PoiService.h"
#include "utils/GeoJsonUtil.h"

namespace controllers
{

    static drogon::HttpResponsePtr
    makeJsonError(const std::string& message, drogon::HttpStatusCode code = drogon::k400BadRequest)
    {
        Json::Value err;
        err["error"] = message;
        auto resp    = drogon::HttpResponse::newHttpJsonResponse(err);
        return resp;
    }

    static drogon::HttpResponsePtr makeJsonMessage(const std::string&     message,
                                                   drogon::HttpStatusCode code = drogon::k200OK)
    {
        Json::Value out;
        out["message"] = message;
        auto resp      = drogon::HttpResponse::newHttpJsonResponse(out);
        resp->setStatusCode(code);
        return resp;
    }

    auto PoiController::create(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto body = req->getJsonObject();
        if (!body || !body->isMember("mapSourceId") || !(*body)["mapSourceId"].isInt())
        {
            Json::Value err;
            err["status"]  = "error";
            err["message"] = "mapSourceId (int) is required";
            auto resp      = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k400BadRequest);
            co_return resp;
        }

        if (!body || !body->isMember("path"))
        {
            Json::Value err;
            err["status"]  = "error";
            err["message"] = "Path to .osm.pbf file is required";
            auto resp      = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k400BadRequest);
            co_return resp;
        }

        int         mapSourceId = (*body)["mapSourceId"].asInt();
        std::string path        = (*body)["path"].asString();

        LOG_INFO << "Executing scripts/extract_poi.sh";

        std::string cmd = "bash /app/scripts/extract_poi.sh \"" + path + "\"";

        std::filesystem::path inputPath(path);

        std::filesystem::path baseDir = inputPath.parent_path();

        std::filesystem::path poiDir = baseDir / "poi";
        std::filesystem::create_directories(poiDir);

        std::filesystem::path geojsonFilePath = poiDir / "poi.geojson";
        std::string           geojsonFile     = geojsonFilePath.string();

        int ret = system(cmd.c_str());
        if (ret != 0)
        {
            Json::Value err;
            err["status"]  = "error";
            err["message"] = "Failed to execute scripts/extract_poi.sh";
            co_return drogon::HttpResponse::newHttpJsonResponse(err);
        }

        std::ifstream file(geojsonFile);
        if (!file.is_open())
        {
            Json::Value err;
            err["status"]  = "error";
            err["message"] = "GeoJSON file not found: " + geojsonFile;
            co_return drogon::HttpResponse::newHttpJsonResponse(err);
        }

        std::string geojsonStr((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());

        if (geojsonStr.empty())
        {
            Json::Value err;
            err["status"]  = "error";
            err["message"] = "GeoJSON file is empty: " + geojsonFile;
            co_return drogon::HttpResponse::newHttpJsonResponse(err);
        }

        LOG_INFO << "Successfully read GeoJSON from " << geojsonFile;

        auto dtos = utils::parseGeoJsonToCreateDtos(geojsonStr);

        for (auto& d : dtos)
            d.mapSourceId = mapSourceId;

        auto service = drogon::app().getPlugin<services::PoiService>();

        std::vector<int> insertedIds;
        try
        {
            insertedIds = co_await service->createPois(std::move(dtos));
        }
        catch (const std::exception& e)
        {
            Json::Value err;
            err["status"]  = "error";
            err["message"] = std::string("Internal error: ") + e.what();
            co_return drogon::HttpResponse::newHttpJsonResponse(err);
        }

        Json::Value resp;
        resp["status"] = "ok";
        for (int id : insertedIds)
            resp["inserted_ids"].append(id);

        auto httpResp = drogon::HttpResponse::newHttpJsonResponse(resp);
        httpResp->setStatusCode(drogon::k201Created);
        co_return httpResp;
    }

    auto PoiController::getOne(drogon::HttpRequestPtr req,
                               int                    id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto poiService = drogon::app().getPlugin<services::PoiService>();
            auto maybePoi   = co_await poiService->getPoiById(id);

            if (!maybePoi)
                co_return makeJsonError("POI not found", drogon::k404NotFound);

            Json::Value body = maybePoi->toJson();
            auto        resp = drogon::HttpResponse::newHttpJsonResponse(body);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[POI CONTROLLER] Error (getOne): " << e.what();
            co_return makeJsonError("Internal Error", drogon::k500InternalServerError);
        }
    }

    auto
    PoiController::getFiltered(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            int type_id = std::stoi(req->getParameter("type_id"));

            auto poiService = drogon::app().getPlugin<services::PoiService>();
            auto pois       = co_await poiService->getPoiByType(type_id);

            if (pois.empty())
                co_return makeJsonError("POI with that filter not found", drogon::k404NotFound);

            Json::Value arr(Json::arrayValue);

            for (const auto& poi : pois)
            {
                arr.append(poi.toJson());
            }

            auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[POI CONTROLLER] Error (getFiltered): " << e.what();
            co_return makeJsonError("Internal Error", drogon::k500InternalServerError);
        }
    }

}  // namespace controllers