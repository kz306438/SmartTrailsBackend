#include "controllers/RouteController.h"

#include "dto/RequestRouteDto.h"
#include "services/RouteService.h"
#include "utils/JsonResponseUtil.h"

namespace controllers
{

    auto RouteController::getAll(drogon::HttpRequestPtr req,
                                 int user_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
    }

    auto RouteController::getOne(drogon::HttpRequestPtr req, int user_id,
                                 int route_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
    }

    // {
    //     "user_id",
    //     "name",
    //     "start_point",
    //     "distance",
    //     "poi_types": [8,9,11],
    // }
    auto RouteController::generate(drogon::HttpRequestPtr req,
                                   int user_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto json = req->getJsonObject();
        if (!json)
            co_return utils::makeJsonError("Body connot be empty!");

        if (user_id <= 0)
            co_return utils::makeJsonError("Invalid user id!");

        if (!json->isMember("name"))
            co_return utils::makeJsonError("Missing 'name' in request body");

        if (!json->isMember("start_point"))
            co_return utils::makeJsonError("Missing 'start_point' in request body");

        if (!json->isMember("distance"))
            co_return utils::makeJsonError("Missing 'distance' in request body");

        if (!json->isMember("poi_types"))
            co_return utils::makeJsonError("Missing 'poi_types' in request body");

        const auto& arr = (*json)["poi_types"];
        if (!arr.isArray())
            utils::makeJsonError("poi_types must be array");

        std::vector<int> poi_types;
        poi_types.reserve(arr.size());
        for (const auto& elem : arr)
        {
            if (!elem.isInt())
                utils::makeJsonError("poi_types must contain integers");
            poi_types.push_back(elem.asInt());
        }

        dto::RequestRouteDto dto;
        dto.user_id     = user_id;
        dto.name        = (*json)["name"].asString();
        dto.start_point = (*json)["start_point"].asString();
        dto.distance    = (*json)["distance"].asDouble();
        dto.poi_types   = std::move(poi_types);

        auto service = drogon::app().getPlugin<services::RouteService>();

        co_await service->createRoute(dto);
    }

    auto RouteController::updateOne(drogon::HttpRequestPtr req, int user_id,
                                    int route_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
    }

    auto RouteController::deleteOne(drogon::HttpRequestPtr req, int user_id,
                                    int route_id) -> drogon::Task<drogon::HttpResponsePtr>
    {
    }

}  // namespace controllers