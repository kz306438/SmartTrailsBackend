#pragma once

#include <drogon/HttpController.h>

namespace controllers
{

    class UserController : public drogon::HttpController<UserController>
    {
      public:
        METHOD_LIST_BEGIN

        ADD_METHOD_TO(UserController::getMe, "/api/users/me", drogon::Get,
                      "filters::AuthenticationFilter");
        ADD_METHOD_TO(UserController::updateMe, "/api/users/me", drogon::Put,
                      "filters::AuthenticationFilter");
        ADD_METHOD_TO(UserController::deleteMe, "/api/users/me", drogon::Delete,
                      "filters::AuthenticationFilter");

        ADD_METHOD_TO(UserController::getAll, "/api/users", drogon::Get,
                      "filters::AuthenticationFilter", "filters::AdminFilter");
        ADD_METHOD_TO(UserController::getOne, "/api/users/{id}", drogon::Get,
                      "filters::AuthenticationFilter", "filters::AdminFilter");
        ADD_METHOD_TO(UserController::deleteOne, "/api/users/{id}", drogon::Delete,
                      "filters::AuthenticationFilter", "filters::AdminFilter");
        ADD_METHOD_TO(UserController::updateRole, "/api/users/{id}/role", drogon::Patch,
                      "filters::AuthenticationFilter", "filters::AdminFilter");

        METHOD_LIST_END

      public:
        auto getMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto updateMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto deleteMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;

        auto getAll(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto getOne(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto deleteOne(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
        auto updateRole(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
    };

}  // namespace controllers
