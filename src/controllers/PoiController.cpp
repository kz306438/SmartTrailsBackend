#include "controllers/PoiController.h"

#include <filesystem>
#include <fstream>

#include "services/PoiService.h"
#include "utils/GeoJsonUtil.h"

namespace controllers
{

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

    auto PoiController::getOne(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
    }

    auto
    PoiController::getFiltered(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
    }

}  // namespace controllers