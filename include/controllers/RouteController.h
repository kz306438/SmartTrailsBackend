#pragma once
#include <drogon/HttpController.h>

namespace controllers
{

    class RouteController : public drogon::HttpController<RouteController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(RouteController::getAll, "/api/routes", drogon::Get,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::getOne, "/api/routes/{id}", drogon::Get,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::generate, "/api/routes", drogon::Post,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::updateOne, "/api/routes/{id}", drogon::Patch,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(RouteController::deleteOne, "/api/routes/{id}", drogon::Delete,
                      "filters::AuthenticationFilter");

        METHOD_LIST_END

      public:
        auto getAll(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto getOne(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto generate(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto updateOne(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto deleteOne(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
    };

}  // namespace controllers