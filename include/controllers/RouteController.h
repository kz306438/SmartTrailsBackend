#pragma once
#include <drogon/HttpController.h>
#include <json/json.h>

#include "dto/RequestRouteDto.h"

namespace controllers
{

    class RouteController : public drogon::HttpController<RouteController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(RouteController::getAll, "/api/users/{user_id}/routes", drogon::Get,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::getOne, "/api/users/{user_id}/routes/{route_id}",
                      drogon::Get, "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::generate, "/api/users/{user_id}/routes", drogon::Post,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::updateOne, "/api/users/{user_id}/routes/{route_id}",
                      drogon::Patch, "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::deleteOne, "/api/users/{user_id}/routes/{route_id}",
                      drogon::Delete, "filters::AuthenticationFilter");

        METHOD_LIST_END

      public:
        auto getAll(drogon::HttpRequestPtr req,
                    int                    user_id) -> drogon::Task<drogon::HttpResponsePtr>;

        auto getOne(drogon::HttpRequestPtr req, int user_id,
                    int route_id) -> drogon::Task<drogon::HttpResponsePtr>;

        auto generate(drogon::HttpRequestPtr req,
                      int                    user_id) -> drogon::Task<drogon::HttpResponsePtr>;

        auto updateOne(drogon::HttpRequestPtr req, int user_id,
                       int route_id) -> drogon::Task<drogon::HttpResponsePtr>;

        auto deleteOne(drogon::HttpRequestPtr req, int user_id,
                       int route_id) -> drogon::Task<drogon::HttpResponsePtr>;

      private:
        auto parseAndValidateGenerateRequest(const std::shared_ptr<Json::Value>& json, int user_id)
            -> std::pair<std::optional<dto::RequestRouteDto>, std::string>;

        auto fetchRouteWithGeoJson(int route_id) -> drogon::Task<std::optional<Json::Value>>;
    };

}  // namespace controllers