#include "controllers/MapSourceController.h"

#include "services/MapSourceService.h"
#include "utils/JsonResponseUtil.h"

namespace controllers
{

    auto MapSourceController::createMapSource(drogon::HttpRequestPtr req)
        -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto body = req->getJsonObject();

            if (!body || !body->isMember("url"))
                co_return utils::makeJsonError("URL to resource is required");

            if (!body->isMember("name"))
                co_return utils::makeJsonError("MapSource name is required");

            std::string url  = (*body)["url"].asString();
            std::string name = (*body)["name"].asString();

            LOG_INFO << "Executing scripts/download_map.sh";

            std::string cmd = "bash /app/scripts/download_map.sh \"" + url + "\"";

            int ret = system(cmd.c_str());
            if (ret != 0)
                co_return utils::makeJsonMessage("Failed to execute scripts/download_map.sh");

            auto service      = drogon::app().getPlugin<services::MapSourceService>();
            auto mapSourceOpt = co_await service->createMapSource(name, url);

            if (!mapSourceOpt)
                co_return utils::makeJsonError("Map source is not created",
                                               drogon::k500InternalServerError);

            co_return drogon::HttpResponse::newHttpJsonResponse(mapSourceOpt->toJson());
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[MAP-SOURCE CONTROLLER] Error (createMapSource): " << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto MapSourceController::getAllMapSources(drogon::HttpRequestPtr req)
        -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto service    = drogon::app().getPlugin<services::MapSourceService>();
            auto mapSources = co_await service->getAllMapSources();

            if (mapSources.empty())
                co_return utils::makeJsonMessage("Map resources are not found",
                                                 drogon::k404NotFound);

            Json::Value arr(Json::arrayValue);
            for (const auto& mapSource : mapSources)
            {
                arr.append(mapSource.toJson());
            }

            auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[MAP-SOURCE CONTROLLER] Error (getAllMapSources): " << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto MapSourceController::deleteMapSource(drogon::HttpRequestPtr req,
                                              int id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto service   = drogon::app().getPlugin<services::MapSourceService>();
            bool isDeleted = co_await service->deleteMapSource(id);

            if (!isDeleted)
                co_return utils::makeJsonError("Map source not found or delete failed",
                                               drogon::k500InternalServerError);

            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k204NoContent);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[MAP-SOURCE CONTROLLER] Error (deleteMapSource): " << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto MapSourceController::setStatus(drogon::HttpRequestPtr req,
                                        int id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto json = req->getJsonObject();
            if (!json || !json->isMember("status"))
                co_return utils::makeJsonError("Missing 'status' in request body",
                                               drogon::k400BadRequest);

            if (!json->isMember("path"))
                co_return utils::makeJsonError("Missing 'path' in request body",
                                               drogon::k400BadRequest);

            std::string newStatus = (*json)["status"].asString();
            std::string path      = (*json)["path"].asString();

            if (path.empty())
                co_return utils::makeJsonError("Field path is empty", drogon::k400BadRequest);

            if (newStatus.empty())
                co_return utils::makeJsonError("Field status is empty", drogon::k400BadRequest);

            if (newStatus != "true" && newStatus != "false")
                co_return utils::makeJsonError("Invalid status", drogon::k400BadRequest);

            auto service      = drogon::app().getPlugin<services::MapSourceService>();
            auto mapSourceOpt = co_await service->getMapSourceById(id);
            if (!mapSourceOpt)
                co_return utils::makeJsonError("Map source not found", drogon::k404NotFound);

            mapSourceOpt->setIsActive((newStatus == "true") ? true : false);

            auto ok = co_await service->updateMapSource(mapSourceOpt.value(), path);
            if (!ok)
                co_return utils::makeJsonError("Failed to update map source",
                                               drogon::k500InternalServerError);

            co_return utils::makeJsonMessage("Map source updated", drogon::k200OK);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[MAP-SOURCE CONTROLLER] error (setStatus)" << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto MapSourceController::cropMap(drogon::HttpRequestPtr req,
                                      int id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto body = req->getJsonObject();
            if (!body)
                co_return utils::makeJsonError("JSON body is required");

            // ----------- Validate required parameters ----------
            static const std::vector<std::string> required = {"path",   "name",  "left",
                                                              "bottom", "right", "top"};

            for (const auto& f : required)
            {
                if (!body->isMember(f))
                    co_return utils::makeJsonError("Field '" + f + "' is required");
            }

            std::string path   = (*body)["path"].asString();
            std::string name   = (*body)["name"].asString();
            double      left   = (*body)["left"].asDouble();
            double      bottom = (*body)["bottom"].asDouble();
            double      right  = (*body)["right"].asDouble();
            double      top    = (*body)["top"].asDouble();

            // ------------------- Execute script -------------------
            std::stringstream cmd;
            cmd << "bash /app/scripts/crop_map.sh " << "\"" << path << "\" " << left << " "
                << bottom << " " << right << " " << top;

            LOG_INFO << "Executing: " << cmd.str();

            int ret = system(cmd.str().c_str());
            if (ret != 0)
                co_return utils::makeJsonError("Failed to run crop_map.sh",
                                               drogon::k500InternalServerError);

            // ---------- Build output .osm.pbf path -----------
            // Example:
            // path = /app/maps/europe/belarus/belarus-latest.osm.pbf
            // result = /app/maps/europe/belarus/cropped/belarus-latest_cropped.osm.pbf

            std::filesystem::path input(path);
            auto                  parent = input.parent_path();
            auto                  stem   = input.stem().string();  // "belarus-latest"
            auto                  out    = parent / "cropped" / (stem + "_cropped.osm.pbf");

            std::string outputPath = out.string();

            // ------------------ Save in DB ---------------------
            auto service      = drogon::app().getPlugin<services::MapSourceService>();
            auto mapSourceOpt = co_await service->createMapSource(name, outputPath);

            if (!mapSourceOpt)
                co_return utils::makeJsonError("Failed to create map source",
                                               drogon::k500InternalServerError);

            co_return drogon::HttpResponse::newHttpJsonResponse(mapSourceOpt->toJson());
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[MAP-SOURCE] Error: " << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

}  // namespace controllers