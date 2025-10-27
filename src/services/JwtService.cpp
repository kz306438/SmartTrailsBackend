#include "services/JwtService.h"

#include <jwt-cpp/jwt.h>

#include <chrono>
#include <iostream>

namespace services
{

    using namespace std::chrono;

    auto JwtService::initAndStart(const Json::Value& config) -> void
    {
        if (config.isMember("secret"))
            secret_ = config["secret"].asString();
        if (config.isMember("issuer"))
            issuer_ = config["issuer"].asString();
        if (config.isMember("expiration"))
            expiration_ = config["expiration"].asInt();

        if (secret_.empty())
            secret_ = "default_secret";
        if (issuer_.empty())
            issuer_ = "app";

        std::cout << "[JwtService] Initialized. issuer=" << issuer_
                  << ", expiretion=" << expiration_ << "s\n";
    }

    auto JwtService::generateToken(const std::string& userId,
                                   const std::string& role) -> std::string const
    {
        if (role.empty())
            throw std::invalid_argument("Role must not be empty when generating JWT");

        const auto now = system_clock::now();
        const auto iat = duration_cast<seconds>(now.time_since_epoch()).count();
        const auto exp = iat + expiration_;

        auto token = jwt::create()
                         .set_issuer(issuer_)
                         .set_subject(userId)
                         .set_issued_at(system_clock::from_time_t(iat))
                         .set_expires_at(system_clock::from_time_t(exp))
                         .set_payload_claim("role", jwt::claim(role))
                         .sign(jwt::algorithm::hs256{secret_});

        return token;
    }

    auto
    JwtService::verifyAndDecode(const std::string& token) -> std::optional<DecodedJWTData> const
    {
        try
        {
            auto decoded = jwt::decode(token);

            auto verifier =
                jwt::verify().allow_algorithm(jwt::algorithm::hs256{secret_}).with_issuer(issuer_);

            verifier.verify(decoded);

            if (!decoded.has_subject() || !decoded.has_payload_claim("role"))
            {
                LOG_WARN << "[JwtService] Missing required claims\n";
                return std::nullopt;
            }

            DecodedJWTData data;
            data.userId = decoded.get_subject();
            data.role   = decoded.get_payload_claim("role").as_string();

            return data;
        }
        catch (const std::exception& e)
        {
            LOG_WARN << "[JwtService] verifyAndDecode failed: " << e.what() << '\n';
            return std::nullopt;
        }
    }
}  // namespace services