#include "controllers/AuthController.h"

#include <drogon/drogon.h>

#include "services/AuthService.h"
#include "utils/JsonResponseUtil.h"

namespace controllers
{
    auto AuthController::registerUser(drogon::HttpRequestPtr req)
        -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto json = req->getJsonObject();
        if (!json)
            co_return utils::makeJsonError("Invalid JSON", drogon::k400BadRequest);

        if (!json->isMember("username") || !json->isMember("email") || !json->isMember("password"))
            co_return utils::makeJsonError("Missing fields", drogon::k400BadRequest);

        auto username = (*json)["username"].asString();
        auto email    = (*json)["email"].asString();
        auto password = (*json)["password"].asString();

        auto authService = drogon::app().getPlugin<services::AuthService>();

        auto result = co_await authService->registerUser(username, email, password);

        if (!result.isOk())
        {
            co_return utils::fromServiceError(result.errorType, result.errorMessage,
                                              result.errorField);
        }

        Json::Value respJson;
        respJson["token"] = *result.data;
        auto resp         = drogon::HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k201Created);
        co_return resp;
    }

    auto
    AuthController::loginUser(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto json = req->getJsonObject();
        if (!json)
        {
            auto resp = drogon::HttpResponse::newHttpJsonResponse({{"error", "Invalid JSON"}});
            resp->setStatusCode(drogon::k400BadRequest);
            co_return resp;
        }

        auto email    = (*json)["email"].asString();
        auto password = (*json)["password"].asString();

        auto authService = drogon::app().getPlugin<services::AuthService>();
        auto result      = co_await authService->loginUser(email, password);

        if (!result.isOk())
        {
            co_return utils::fromServiceError(result.errorType, result.errorMessage);
        }

        Json::Value respJson;
        respJson["token"] = *result.data;
        co_return drogon::HttpResponse::newHttpJsonResponse(respJson);
    }
}  // namespace controllers