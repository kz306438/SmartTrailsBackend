#pragma once
#include <drogon/HttpController.h>

namespace controllers
{

    class AuthController : public drogon::HttpController<AuthController>
    {
      public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::registerUser, "/register", drogon::Post);
        ADD_METHOD_TO(AuthController::loginUser, "/login", drogon::Post);
        METHOD_LIST_END

        auto registerUser(const drogon::HttpRequestPtr&                         req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback) -> void;
        auto loginUser(const drogon::HttpRequestPtr&                         req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) -> void;
    };

}  // namespace controllers