#include "utils/JsonResponseUtil.h"

namespace utils
{

    auto makeJsonError(const std::string&     message,
                       drogon::HttpStatusCode code) -> drogon::HttpResponsePtr
    {
        Json::Value err;
        err["error"] = message;
        auto resp    = drogon::HttpResponse::newHttpJsonResponse(err);
        return resp;
    }

    auto makeJsonMessage(const std::string&     message,
                         drogon::HttpStatusCode code) -> drogon::HttpResponsePtr
    {
        Json::Value out;
        out["message"] = message;
        auto resp      = drogon::HttpResponse::newHttpJsonResponse(out);
        resp->setStatusCode(code);
        return resp;
    }

}  // namespace utils