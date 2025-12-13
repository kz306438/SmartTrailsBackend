#pragma once

#include <drogon/plugins/Plugin.h>

#include <memory>
#include <optional>
#include <string>

#include "models/Users.h"
#include "repositories/UserRepository.h"
#include "services/JwtService.h"
#include "utils/ServiceResult.h"

namespace services
{

    class AuthService : public drogon::Plugin<AuthService>
    {
      public:
        void initAndStart(const Json::Value& config) override;

        void shutdown() override;

      public:
        [[nodiscard]]
        auto registerUser(const std::string& username, const std::string& email,
                          const std::string& password)
            -> drogon::Task<utils::ServiceResult<std::string>>;

        [[nodiscard]]
        auto loginUser(const std::string& email, const std::string& password)
            -> drogon::Task<utils::ServiceResult<std::string>>;

      private:
        auto hashPassword(const std::string& password) -> std::string;
        auto verifyPassword(const std::string& password, const std::string& hash) -> bool;

      private:
        std::unique_ptr<repositories::UserRepository> userRepo_;
    };

}  // namespace services