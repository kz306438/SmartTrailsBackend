#include "services/UserService.h"

#include "drogon/drogon.h"

namespace services
{

    auto UserService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        userRepo_     = std::make_unique<repositories::UserRepository>(dbClient);
        LOG_INFO << "[USER SERVICE] Plugin started";
    }

    auto UserService::shutdown() -> void
    {
        userRepo_.reset();
        LOG_INFO << "[USER SERVICE] Plugin stopped";
    }

    auto UserService::createUser(const std::string& username, const std::string& email,
                                 const std::string& passwordHash, const std::string& role)
        -> drogon::Task<std::optional<repositories::models::Users>>
    {
        if (co_await userRepo_->getUserByEmail(email))
        {
            LOG_WARN << "[AUTH SERVICE] Error (createUser:getUserByEmail): " << email;
            co_return std::nullopt;
        }
        if (co_await userRepo_->getUserByUsername(username))
        {
            LOG_WARN << "[AUTH SERVICE] Error (createUser:getUserByUsername): " << username;
            co_return std::nullopt;
        }

        auto user = co_await userRepo_->createUser(username, email, passwordHash, role);
        co_return user;
    }

    auto
    UserService::getUserById(int userId) -> drogon::Task<std::optional<repositories::models::Users>>
    {
        co_return co_await userRepo_->getUserById(userId);
    }

    auto UserService::getUserByEmail(const std::string& email)
        -> drogon::Task<std::optional<repositories::models::Users>>
    {
        co_return co_await userRepo_->getUserByEmail(email);
    }

    auto UserService::getAllUsers() -> drogon::Task<std::vector<repositories::models::Users>>
    {
        auto allUsers = co_await userRepo_->getAllUsers();
        co_return allUsers;
    }

    auto UserService::updateUser(const repositories::models::Users& user) -> drogon::Task<bool>
    {
        co_return co_await userRepo_->updateUser(user);
    }

    auto UserService::deleteUser(int userId) -> drogon::Task<bool>
    {
        co_return co_await userRepo_->deleteUser(userId);
    }

    auto UserService::updateUserRole(int userId, const std::string& newRole) -> drogon::Task<bool>
    {
        auto userOpt = co_await userRepo_->getUserById(userId);
        if (!userOpt)
        {
            LOG_WARN << "[AUTH SERVICE] Error (updateUserRole:getUserById): " << userId;
            co_return false;
        }
        auto user = userOpt.value();
        user.setRole(newRole);
        co_return co_await userRepo_->updateUser(user);
    }

}  // namespace services