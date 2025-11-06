#include "filters/AuthenticationFilter.h"

#include <drogon/drogon.h>

#include "services/JwtService.h"

namespace filters
{

    using namespace drogon;

    auto AuthenticationFilter::doFilter(const HttpRequestPtr& req, FilterCallback&& fcb,
                                        FilterChainCallback&& fccb) -> void
    {
        const std::string authHeader = req->getHeader("Authorization");

        auto makeError = [&](const std::string& message, HttpStatusCode code = k401Unauthorized)
        {
            Json::Value error;
            error["error"] = message;
            auto resp      = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(code);
            return resp;
        };

        if (authHeader.empty() || authHeader.rfind("Bearer ", 0) != 0)
        {
            fcb(makeError("Unauthorized"));
            return;
        }

        const std::string token      = authHeader.substr(7);
        auto              jwtService = drogon::app().getPlugin<services::JwtService>();

        try
        {
            auto decoded = jwtService->verifyAndDecode(token);
            if (!decoded)
            {
                fcb(makeError("Invalid or expired token"));
                return;
            }

            req->attributes()->insert("user_id", decoded->userId);
            req->attributes()->insert("role", decoded->role);
            fccb();
        }
        catch (const std::exception& e)
        {
            fcb(makeError(e.what()));
        }
    }

}  // namespace filters
