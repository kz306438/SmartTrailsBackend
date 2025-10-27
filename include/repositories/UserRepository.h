#pragma once

#include <drogon/orm/DbClient.h>

#include <optional>

#include "models/Users.h"

namespace repositories
{

    namespace models = drogon_model::smarttrails;

    class UserRepository
    {
      public:
        explicit UserRepository(drogon::orm::DbClientPtr dbClient);

      public:
        auto createUser(const std::string& username, const std::string& email,
                        const std::string& passwordHash,
                        const std::string& role = "user") -> std::optional<models::Users>;

        [[nodiscard]] auto getUserById(int userId) -> std::optional<models::Users>;

        [[nodiscard]] auto getUserByEmail(const std::string& email) -> std::optional<models::Users>;

        [[nodiscard]] auto
        getUserByUsername(const std::string& username) -> std::optional<models::Users>;

        auto updateUser(const models::Users& user) -> bool;
        auto deleteUser(int userId) -> bool;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };

}  // namespace repositories