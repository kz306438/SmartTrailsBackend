#include "services/AuthService.h"

#include <argon2.h>
#include <drogon/drogon.h>

#include <random>
#include <stdexcept>

namespace services
{

    auto AuthService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        userRepo_     = std::make_unique<repositories::UserRepository>(dbClient);
        LOG_INFO << "[AUTH SERVICE] Plugin started";
    }

    auto AuthService::shutdown() -> void
    {
        userRepo_.reset();
        LOG_INFO << "[AUTH SERVICE] Plugin stopped";
    }

    auto AuthService::hashPassword(const std::string& password) -> std::string
    {
        char          hash[128];
        unsigned char salt[16];

        std::random_device rd;
        for (unsigned char& c : salt)
            c = static_cast<unsigned char>(rd() & 0xFF);

        int result = argon2id_hash_encoded(3,        // Iterations
                                           1 << 16,  // Memory 64MB
                                           1,        // Threads
                                           password.c_str(), password.size(), salt, sizeof(salt),
                                           32, hash, sizeof(hash));

        if (result != ARGON2_OK)
            throw std::runtime_error("Argon2 hashing failed");

        return std::string(hash);
    }

    auto AuthService::verifyPassword(const std::string& password, const std::string& hash) -> bool
    {
        int result = argon2id_verify(hash.c_str(), password.c_str(), password.size());
        return result == ARGON2_OK;
    }

    auto AuthService::registerUser(const std::string& username, const std::string& email,
                                   const std::string& password)
        -> drogon::Task<std::optional<std::string>>
    {
        auto byEmail = co_await userRepo_->getUserByEmail(email);
        auto byName  = co_await userRepo_->getUserByUsername(username);
        if (byEmail || byName)
        {
            LOG_WARN << "[AUTH SERVICE] User already exists: " << email;
            co_return std::nullopt;
        }

        try
        {
            auto hashed  = hashPassword(password);
            auto newUser = co_await userRepo_->createUser(username, email, hashed, "user");
            if (!newUser)
                co_return std::nullopt;

            auto jwtService = drogon::app().getPlugin<JwtService>();
            auto token =
                jwtService->generateToken(newUser->getValueOfId(), newUser->getValueOfRole());

            co_return token;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[AUTH SERVICE] Registration failed: " << e.what();
            co_return std::nullopt;
        }
    }

    auto AuthService::loginUser(const std::string& email, const std::string& password)
        -> drogon::Task<std::optional<std::string>>
    {
        auto user = co_await userRepo_->getUserByEmail(email);
        if (!user)
            co_return std::nullopt;

        if (!verifyPassword(password, user->getValueOfPasswordHash()))
            co_return std::nullopt;

        auto jwtService = drogon::app().getPlugin<JwtService>();
        co_return jwtService->generateToken(user->getValueOfId(), user->getValueOfRole());
    }

}  // namespace services