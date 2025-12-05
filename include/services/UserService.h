#pragma once

#include <drogon/plugins/Plugin.h>

#include <optional>
#include <string>

#include "repositories/UserRepository.h"

namespace services
{

    class UserService : public drogon::Plugin<UserService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

      public:
        [[nodiscard]]
        auto createUser(const std::string& username, const std::string& email,
                        const std::string& passwordHash, const std::string& role = "user")
            -> drogon::Task<std::optional<repositories::models::Users>>;

        [[nodiscard]]
        auto getUserById(int userId) -> drogon::Task<std::optional<repositories::models::Users>>;

        [[nodiscard]]
        auto getUserByEmail(const std::string& email)
            -> drogon::Task<std::optional<repositories::models::Users>>;

        [[nodiscard]]
        auto getAllUsers() -> drogon::Task<std::vector<repositories::models::Users>>;

      public:
        auto updateUser(const repositories::models::Users& user) -> drogon::Task<bool>;
        auto deleteUser(int userId) -> drogon::Task<bool>;
        auto updateUserRole(int userId, const std::string& newRole) -> drogon::Task<bool>;

      private:
        std::unique_ptr<repositories::UserRepository> userRepo_;
    };

}  // namespace services
