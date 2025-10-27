#pragma once
#include <drogon/plugins/Plugin.h>

#include <optional>
#include <string>

namespace services
{

    struct DecodedJWTData
    {
        std::string userId;
        std::string role;
    };

    class JwtService : public drogon::Plugin<JwtService>
    {
      public:
        JwtService() = default;

        auto initAndStart(const Json::Value& config) -> void override;
        auto shutdown() -> void override {}

        [[nodiscard]] auto generateToken(const std::string& userId,
                                         const std::string& role) -> std::string const;
        [[nodiscard]] auto
        verifyAndDecode(const std::string& token) -> std::optional<DecodedJWTData> const;

      private:
        std::string secret_;
        std::string issuer_;
        long        expiration_;
    };

}  // namespace services
