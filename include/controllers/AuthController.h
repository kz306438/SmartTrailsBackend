#pragma once
#include <drogon/HttpController.h>

namespace controllers
{

    class AuthController : public drogon::HttpController<AuthController>
    {
      public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::registerUser, "/auth/register", drogon::Post);
        ADD_METHOD_TO(AuthController::loginUser, "/auth/login", drogon::Post);
        METHOD_LIST_END

      public:
        auto registerUser(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto loginUser(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
    };

}  // namespace controllers