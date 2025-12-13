#pragma once

#include <drogon/HttpController.h>

#include "utils/ServiceResult.h"

namespace utils
{

    auto
    makeJsonError(const std::string&     message,
                  drogon::HttpStatusCode code = drogon::k400BadRequest) -> drogon::HttpResponsePtr;

    auto makeJsonMessage(const std::string&     message,
                         drogon::HttpStatusCode code = drogon::k200OK) -> drogon::HttpResponsePtr;

    drogon::HttpResponsePtr fromServiceError(ServiceErrorType type, const std::string& msg,
                                             const std::string& field = "");

}  // namespace utils