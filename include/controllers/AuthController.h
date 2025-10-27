#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class AuthController : public drogon::HttpController<AuthController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::registerUser, "/register", Post);
    ADD_METHOD_TO(AuthController::loginUser, "/login", Post);
    METHOD_LIST_END

    auto registerUser(const HttpRequestPtr&                         req,
                      std::function<void(const HttpResponsePtr&)>&& callback) -> void;
    auto loginUser(const HttpRequestPtr&                         req,
                   std::function<void(const HttpResponsePtr&)>&& callback) -> void;
};
