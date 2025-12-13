#include "controllers/SystemController.h"
#include "services/SystemService.h"
#include "utils/JsonResponseUtil.h"

namespace controllers
{

    auto SystemController::getMetrics(drogon::HttpRequestPtr req)
        -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto service = drogon::app().getPlugin<services::SystemService>();
            auto metrics = co_await service->getMetrics();

            Json::Value json;
            json["cpu"]    = metrics.cpuUsagePercent;
            json["memory"] = metrics.memoryUsagePercent;
            json["users"]  = metrics.activeUsers;
            json["status"] = "Online";  // Static for now

            co_return drogon::HttpResponse::newHttpJsonResponse(json);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "SystemController error: " << e.what();
            co_return utils::makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }
}  // namespace controllers