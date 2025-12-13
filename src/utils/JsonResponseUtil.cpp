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

    drogon::HttpResponsePtr fromServiceError(ServiceErrorType type, const std::string& msg,
                                             const std::string& field)
    {
        Json::Value json;
        json["error"] = msg;
        if (!field.empty())
        {
            json["field"] = field;
        }

        auto resp = drogon::HttpResponse::newHttpJsonResponse(json);

        switch (type)
        {
            case ServiceErrorType::BadRequest:
                resp->setStatusCode(drogon::k400BadRequest);
                break;
            case ServiceErrorType::NotFound:
                resp->setStatusCode(drogon::k404NotFound);
                break;
            case ServiceErrorType::Conflict:
                resp->setStatusCode(drogon::k409Conflict);
                break;
            case ServiceErrorType::Unauthorized:
                resp->setStatusCode(drogon::k401Unauthorized);
                break;
            case ServiceErrorType::Forbidden:
                resp->setStatusCode(drogon::k403Forbidden);
                break;
            default:
                resp->setStatusCode(drogon::k500InternalServerError);
                break;
        }
        return resp;
    }

}  // namespace utils