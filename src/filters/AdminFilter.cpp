#include "filters/AdminFilter.h"

#include <drogon/drogon.h>

namespace filters
{

    auto AdminFilter::doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback&& fcb,
                               drogon::FilterChainCallback&& fccb) -> void
    {
        try
        {
            const auto& attrs = req->attributes();
            if (attrs->find("role"))
            {
                auto role = attrs->get<std::string>("role");
                if (role == "admin")
                {
                    fccb();
                    return;
                }
                else
                {
                    LOG_WARN << "Access denied for role: " << role;
                }
            }
            else
            {
                LOG_WARN << "No role attribute in request";
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "AdminFilter exception: " << e.what();
        }

        Json::Value error;
        error["error"] = "Forbidden - admin only";

        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k403Forbidden);
        fcb(resp);
    }

}  // namespace filters
