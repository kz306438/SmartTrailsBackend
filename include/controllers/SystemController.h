#pragma once
#include <drogon/HttpController.h>

namespace controllers
{
    class SystemController : public drogon::HttpController<SystemController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(SystemController::getMetrics, "/api/admin/metrics", drogon::Get,
                      "filters::AuthenticationFilter", "filters::AdminFilter");
        METHOD_LIST_END

        auto getMetrics(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
    };
}  // namespace controllers