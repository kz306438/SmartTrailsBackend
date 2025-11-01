#include "repositories/UserRepository.h"

using namespace drogon::orm;

namespace repositories
{

    UserRepository::UserRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

    auto UserRepository::createUser(const std::string& username, const std::string& email,
                                    const std::string& passwordHash, const std::string& role)
        -> drogon::Task<std::optional<models::Users>>
    {
        try
        {
            models::Users user;
            user.setUsername(username);
            user.setEmail(email);
            user.setPasswordHash(passwordHash);
            user.setRole(role);
            user.setCreatedAt(trantor::Date::now());

            CoroMapper<models::Users> usersMapper(dbClient_);
            co_await usersMapper.insert(user);

            co_return user;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createUser): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto UserRepository::getUserById(int userId) -> drogon::Task<std::optional<models::Users>>
    {
        try
        {
            CoroMapper<models::Users> mp(dbClient_);
            auto                      user = co_await mp.findByPrimaryKey(userId);
            co_return user;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getUserId): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto UserRepository::getUserByEmail(const std::string& email)
        -> drogon::Task<std::optional<models::Users>>
    {
        try
        {
            CoroMapper<models::Users> mp(dbClient_);
            auto                      user = co_await mp.findOne(
                Criteria(models::Users::Cols::_email, CompareOperator::EQ, email));
            co_return user;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getUserByEmail): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto UserRepository::getUserByUsername(const std::string& username)
        -> drogon::Task<std::optional<models::Users>>
    {
        try
        {
            CoroMapper<models::Users> mp(dbClient_);
            auto                      user = co_await mp.findOne(
                Criteria(models::Users::Cols::_username, CompareOperator::EQ, username));
            co_return user;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getUserBySurname): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto UserRepository::getAllUsers() -> drogon::Task<std::vector<models::Users>>
    {
        try
        {
            CoroMapper<models::Users> mp(dbClient_);

            auto users = co_await mp.findAll();
            co_return users;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (getAllUsers): " << e.base().what();
            co_return {};
        }
    }

    auto UserRepository::updateUser(const models::Users& user) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::Users> mp(dbClient_);
            co_await mp.update(user);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (updateUser): " << e.base().what();
            co_return false;
        }
    }

    auto UserRepository::deleteUser(int userId) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::Users> mp(dbClient_);
            co_await mp.deleteByPrimaryKey(userId);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (deleteUser): " << e.base().what();
            co_return false;
        }
    }

}  // namespace repositories