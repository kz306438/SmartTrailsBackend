#pragma once
#include <drogon/HttpController.h>

namespace controllers
{

    class PoiTypeController : public drogon::HttpController<PoiTypeController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(PoiTypeController::getPoiTypes, "/api/poi-type", drogon::Get,
                      "filters::AuthenticationFilter");

        METHOD_LIST_END

      public:
        auto getPoiTypes(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
    };

}  // namespace controllers