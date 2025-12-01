#pragma once

#include <drogon/plugins/Plugin.h>

#include <optional>
#include <string>

#include "repositories/UserPreferencesRepository.h"

namespace services
{

    class UserPreferencesService : public drogon::Plugin<UserPreferencesService>
    {
      public:
        auto initAndStart(const Json::Value& config) -> void override;

        auto shutdown() -> void override;

        //   public:
        //     [[nodiscard]]
        //     auto createUser(const std::string& username, const std::string& email,
        //                     const std::string& passwordHash, const std::string& role = "user")
        //         -> drogon::Task<std::optional<repositories::models::UsersPreferences>>;

        // [[nodiscard]]
        // auto getUserById(int userId) -> drogon::Task<std::optional<repositories::models::Users>>;

        // [[nodiscard]]
        // auto getUserByEmail(const std::string& email)
        //     -> drogon::Task<std::optional<repositories::models::Users>>;

        // [[nodiscard]]
        // auto getAllUsers() -> drogon::Task<std::vector<repositories::models::Users>>;

      private:
        std::unique_ptr<repositories::UserPreferencesRepository> userRepo_;
    };

}  // namespace services
