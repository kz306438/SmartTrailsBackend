#pragma once

#include <drogon/HttpController.h>

namespace utils
{

    auto
    makeJsonError(const std::string&     message,
                  drogon::HttpStatusCode code = drogon::k400BadRequest) -> drogon::HttpResponsePtr;

    auto makeJsonMessage(const std::string&     message,
                         drogon::HttpStatusCode code = drogon::k200OK) -> drogon::HttpResponsePtr;

}  // namespace utils