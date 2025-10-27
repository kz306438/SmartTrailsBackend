#pragma once
#include <drogon/HttpFilter.h>

namespace filters
{

    class AdminFilter : public drogon::HttpFilter<AdminFilter>
    {
      public:
        auto doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback&& fcb,
                      drogon::FilterChainCallback&& fccb) -> void override;
    };

}  // namespace filters