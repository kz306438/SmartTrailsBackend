#include "repositories/UserRepository.h"

using namespace drogon::orm;

namespace repositories
{

    UserRepository::UserRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

    auto
    UserRepository::createUser(const std::string& username, const std::string& email,
                               const std::string& passwordHash,
                               const std::string& role = "user") -> std::optional<models::Users>
    {
        try
        {
            models::Users user;
            user.setUsername(username);
            user.setEmail(email);
            user.setPasswordHash(passwordHash);
            user.setRole(role);
            user.setCreatedAt(trantor::Date::now());

            Mapper<models::Users> usersMapper(dbClient_);

            usersMapper.insert(user);

            return user;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createUser): " << e.base().what();
            return std::nullopt;
        }
    }

    auto UserRepository::getUserById(int userId) -> std::optional<models::Users>
    {
        try
        {
            Mapper<models::Users> mp(dbClient_);
            return mp.findByPrimaryKey(userId);
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] User not found by ID: " << e.base().what();
            return std::nullopt;
        }
    }

    auto UserRepository::getUserByEmail(std::string& email) -> std::optional<models::Users>
    {
        try
        {
            Mapper<models::Users> mp(dbClient_);
            return mp.findOne(Criteria(models::Users::Cols::_email, CompareOperator::EQ, email));
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] User not found by username: " << e.base().what();
            return std::nullopt;
        }
    }

    auto UserRepository::getUserByUsername(std::string& username) -> std::optional<models::Users>
    {
        try
        {
            Mapper<models::Users> mp(dbClient_);
            return mp.findOne(
                Criteria(models::Users::Cols::_username, CompareOperator::EQ, username));
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] User not found by username: " << e.base().what();
            return std::nullopt;
        }
    }

    auto UserRepository::updateUser(const models::Users& user) -> bool
    {
        try
        {
            Mapper<models::Users> mp(dbClient_);
            mp.update(user);
            return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error updating user: " << e.base().what();
            return false;
        }
    }

    auto UserRepository::deleteUser(int userId) -> bool
    {
        try
        {
            Mapper<models::Users> mp(dbClient_);
            mp.deleteByPrimaryKey(userId);
            return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error deleting user: " << e.base().what();
            return false;
        }
    }

}  // namespace repositories