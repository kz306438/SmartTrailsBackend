#include "controllers/AuthController.h"

#include <drogon/drogon.h>

using namespace drogon;

auto AuthController::registerUser(const HttpRequestPtr&                         req,
                                  std::function<void(const HttpResponsePtr&)>&& callback) -> void
{
    auto        json = req->getJsonObject();
    Json::Value resp;
    if (!json || !(*json).isMember("email") || !(*json).isMember("password"))
    {
        resp["error"] = "Missing email or password";
        auto res      = HttpResponse::newHttpJsonResponse(resp);
        res->setStatusCode(k400BadRequest);
        return callback(res);
    }

    resp["message"] = "User registration endpoint works!";
    auto res        = HttpResponse::newHttpJsonResponse(resp);
    callback(res);
}

auto AuthController::loginUser(const HttpRequestPtr&                         req,
                               std::function<void(const HttpResponsePtr&)>&& callback) -> void
{
    auto        json = req->getJsonObject();
    Json::Value resp;
    if (!json || !(*json).isMember("email") || !(*json).isMember("password"))
    {
        resp["error"] = "Missing email or password";
        auto res      = HttpResponse::newHttpJsonResponse(resp);
        res->setStatusCode(k400BadRequest);
        return callback(res);
    }

    // пока просто заглушка
    resp["message"] = "Login endpoint works!";
    auto res        = HttpResponse::newHttpJsonResponse(resp);
    callback(res);
}
