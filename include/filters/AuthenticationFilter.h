#pragma once

#include <drogon/HttpFilter.h>

namespace filters
{

    class AuthenticationFilter : public drogon::HttpFilter<AuthenticationFilter>
    {
      public:
        auto doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback&& fcb,
                      drogon::FilterChainCallback&& fccb) -> void override;
    };

}  // namespace filters