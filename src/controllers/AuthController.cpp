#include "controllers/AuthController.h"

#include <drogon/drogon.h>

#include "services/AuthService.h"

using namespace drogon;

namespace controllers
{

    void AuthController::registerUser(const HttpRequestPtr&                         req,
                                      std::function<void(const HttpResponsePtr&)>&& callback)
    {
        auto json = req->getJsonObject();
        if (!json)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Invalid JSON"}});
            resp->setStatusCode(k400BadRequest);
            return callback(resp);
        }

        auto username = (*json)["username"].asString();
        auto email    = (*json)["email"].asString();
        auto password = (*json)["password"].asString();

        if (username.empty() || email.empty() || password.empty())
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Missing fields"}});
            resp->setStatusCode(k400BadRequest);
            return callback(resp);
        }

        auto authService = drogon::app().getPlugin<services::AuthService>();
        auto token       = authService->registerUser(username, email, password);
        if (!token)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Registration failed"}});
            resp->setStatusCode(k400BadRequest);
            return callback(resp);
        }

        Json::Value respJson;
        respJson["token"] = *token;
        auto resp         = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        callback(resp);
    }

    void AuthController::loginUser(const HttpRequestPtr&                         req,
                                   std::function<void(const HttpResponsePtr&)>&& callback)
    {
        auto json = req->getJsonObject();
        if (!json)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Invalid JSON"}});
            resp->setStatusCode(k400BadRequest);
            return callback(resp);
        }

        auto email    = (*json)["email"].asString();
        auto password = (*json)["password"].asString();

        if (email.empty() || password.empty())
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Missing fields"}});
            resp->setStatusCode(k400BadRequest);
            return callback(resp);
        }

        auto authService = drogon::app().getPlugin<services::AuthService>();
        auto token       = authService->loginUser(email, password);
        if (!token)
        {
            auto resp = HttpResponse::newHttpJsonResponse({{"error", "Invalid credentials"}});
            resp->setStatusCode(k401Unauthorized);
            return callback(resp);
        }

        Json::Value respJson;
        respJson["token"] = *token;
        auto resp         = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        callback(resp);
    }

}  // namespace controllers