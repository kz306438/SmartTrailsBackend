#include "controllers/AuthController.h"

#include <drogon/drogon.h>

#include "services/AuthService.h"

using namespace drogon;

namespace controllers
{

    auto AuthController::registerUser(HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto json = req->getJsonObject();
        if (!json)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Invalid JSON"}});
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        auto username = (*json)["username"].asString();
        auto email    = (*json)["email"].asString();
        auto password = (*json)["password"].asString();

        if (username.empty() || email.empty() || password.empty())
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Missing fields"}});
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        auto authService = drogon::app().getPlugin<services::AuthService>();
        auto token       = co_await authService->registerUser(username, email, password);
        if (!token)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Registration failed"}});
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        Json::Value respJson;
        respJson["token"] = *token;
        auto resp         = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    }

    auto AuthController::loginUser(HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        auto json = req->getJsonObject();
        if (!json)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Invalid JSON"}});
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        auto email    = (*json)["email"].asString();
        auto password = (*json)["password"].asString();

        if (email.empty() || password.empty())
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Missing fields"}});
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        auto authService = drogon::app().getPlugin<services::AuthService>();
        auto token       = co_await authService->loginUser(email, password);
        if (!token)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Invalid credentials"}});
            resp->setStatusCode(k401Unauthorized);
            co_return resp;
        }

        Json::Value respJson;
        respJson["token"] = *token;
        auto resp         = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    }

}  // namespace controllers