#pragma once
#include <drogon/HttpController.h>

namespace controllers
{

    class MapSourceController : public drogon::HttpController<MapSourceController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(MapSourceController::createMapSource, "/api/map-sources", drogon::Post,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        ADD_METHOD_TO(MapSourceController::getAllMapSources, "/api/map-sources", drogon::Get,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        ADD_METHOD_TO(MapSourceController::deleteMapSource, "/api/map-sources/{id}", drogon::Delete,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        ADD_METHOD_TO(MapSourceController::setStatus, "/api/map-sources/{id}", drogon::Patch,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        ADD_METHOD_TO(MapSourceController::cropMap, "/api/map-sources/{id}/crop", drogon::Post,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        METHOD_LIST_END

      public:
        auto createMapSource(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto getAllMapSources(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto deleteMapSource(drogon::HttpRequestPtr req,
                             int                    id) -> drogon::Task<drogon::HttpResponsePtr>;
        auto setStatus(drogon::HttpRequestPtr req, int id) -> drogon::Task<drogon::HttpResponsePtr>;
        auto cropMap(drogon::HttpRequestPtr req, int id) -> drogon::Task<drogon::HttpResponsePtr>;
    };

}  // namespace controllers