#pragma once
#include <drogon/HttpController.h>

namespace controllers
{

    class PoiController : public drogon::HttpController<PoiController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(PoiController::create, "/api/poi", drogon::Post,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        ADD_METHOD_TO(PoiController::getOne, "/api/poi/{id}", drogon::Get,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(PoiController::getFiltered, "/api/poi", drogon::Get,
                      "filters::AuthenticationFilter");

        METHOD_LIST_END

      public:
        auto create(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto getOne(drogon::HttpRequestPtr req, int id) -> drogon::Task<drogon::HttpResponsePtr>;
        auto getFiltered(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
    };

}  // namespace controllers