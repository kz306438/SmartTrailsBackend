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
        userRepo_     = std::make_shared<repositories::UserRepository>(dbClient);
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
                                   const std::string& password) -> std::optional<std::string>
    {
        if (userRepo_->getUserByEmail(email) || userRepo_->getUserByUsername(username))
        {
            LOG_WARN << "[AUTH SERVICE] User already exists: " << email;
            return std::nullopt;
        }

        try
        {
            auto hashed  = hashPassword(password);
            auto newUser = userRepo_->createUser(username, email, hashed, "user");
            if (!newUser)
                return std::nullopt;

            auto jwtService = drogon::app().getPlugin<JwtService>();
            auto token      = jwtService->generateToken(std::to_string(newUser->getValueOfId()),
                                                        newUser->getValueOfRole());

            return token;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[AUTH SERVICE] Registration failed: " << e.what();
            return std::nullopt;
        }
    }

    auto AuthService::loginUser(const std::string& email,
                                const std::string& password) -> std::optional<std::string>
    {
        auto user = userRepo_->getUserByEmail(email);
        if (!user)
            return std::nullopt;

        if (!verifyPassword(password, user->getValueOfPasswordHash()))
            return std::nullopt;

        auto jwtService = drogon::app().getPlugin<JwtService>();
        return jwtService->generateToken(std::to_string(user->getValueOfId()),
                                         user->getValueOfRole());
    }

}  // namespace services