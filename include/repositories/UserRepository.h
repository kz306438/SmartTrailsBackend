#pragma once

#include <drogon/orm/CoroMapper.h>
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
        auto
        createUser(const std::string& username, const std::string& email,
                   const std::string& passwordHash,
                   const std::string& role = "user") -> drogon::Task<std::optional<models::Users>>;

        [[nodiscard]] auto getUserById(int userId) -> drogon::Task<std::optional<models::Users>>;

        [[nodiscard]] auto
        getUserByEmail(const std::string& email) -> drogon::Task<std::optional<models::Users>>;

        [[nodiscard]] auto getUserByUsername(const std::string& username)
            -> drogon::Task<std::optional<models::Users>>;

        [[nodiscard]] auto getAllUsers() -> drogon::Task<std::vector<models::Users>>;

        auto updateUser(const models::Users& user) -> drogon::Task<bool>;
        auto deleteUser(int userId) -> drogon::Task<bool>;

      private:
        drogon::orm::DbClientPtr dbClient_;
    };

}  // namespace repositories