#include "controllers/UserController.h"

#include "services/UserService.h"

namespace controllers
{

    static drogon::HttpResponsePtr
    makeJsonError(const std::string& message, drogon::HttpStatusCode code = drogon::k400BadRequest)
    {
        Json::Value err;
        err["error"] = message;
        auto resp    = drogon::HttpResponse::newHttpJsonResponse(err);
        return resp;
    }

    static drogon::HttpResponsePtr makeJsonMessage(const std::string&     message,
                                                   drogon::HttpStatusCode code = drogon::k200OK)
    {
        Json::Value out;
        out["message"] = message;
        auto resp      = drogon::HttpResponse::newHttpJsonResponse(out);
        resp->setStatusCode(code);
        return resp;
    }

    auto UserController::getMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            const auto& attrs = req->attributes();
            if (!attrs->find("user_id"))
                co_return makeJsonError("Unauthorized", drogon::k401Unauthorized);

            auto userId = attrs->get<int>("user_id");

            auto userService = drogon::app().getPlugin<services::UserService>();
            auto maybeUser   = co_await userService->getUserById(userId);

            if (!maybeUser)
                co_return makeJsonError("User not found", drogon::k404NotFound);

            Json::Value body = maybeUser->toJson();
            auto        resp = drogon::HttpResponse::newHttpJsonResponse(body);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[USER CONTROLLER] Error (getMe): " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto
    UserController::updateMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            const auto& attrs = req->attributes();
            if (!attrs->find("user_id"))
                co_return makeJsonError("Unauthorized", drogon::k401Unauthorized);

            auto userId = attrs->get<int>("user_id");

            auto json = req->getJsonObject();
            if (!json)
                co_return makeJsonError("Invalid JSON body", drogon::k400BadRequest);

            auto userService = drogon::app().getPlugin<services::UserService>();
            auto maybeUser   = co_await userService->getUserById(userId);
            if (!maybeUser)
                co_return makeJsonError("User not found", drogon::k404NotFound);

            Json::Value patched = maybeUser->toJson();

            // TO DO: maybe something else
            if (json->isMember("username"))
                patched["username"] = (*json)["username"];

            (*maybeUser).updateByJson(patched);

            auto ok = co_await userService->updateUser(maybeUser.value());
            if (!ok)
                co_return makeJsonError("Failed to update user", drogon::k500InternalServerError);

            co_return makeJsonMessage("User updated", drogon::k200OK);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[USER CONTROLLER] Error (updateMe): " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto
    UserController::deleteMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            const auto& attrs = req->attributes();
            if (!attrs->find("user_id"))
                co_return makeJsonError("Unauthorized", drogon::k401Unauthorized);

            const auto userId = attrs->get<int>("user_id");

            auto userService = drogon::app().getPlugin<services::UserService>();
            auto ok          = co_await userService->deleteUser(userId);
            if (!ok)
                co_return makeJsonError("Failed to delete user", drogon::k500InternalServerError);

            // Successful deletion — 204 No Content
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k204NoContent);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[USER CONTROLLER] Error (deleteMe): " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto UserController::getAll(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto userService = drogon::app().getPlugin<services::UserService>();
            auto users       = co_await userService->getAllUsers();

            Json::Value arr(Json::arrayValue);
            for (const auto& u : users)
            {
                arr.append(u.toJson());
            }

            auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[USER CONTROLLER] Error (getAll): " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto UserController::getOne(drogon::HttpRequestPtr req,
                                int                    id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto userService = drogon::app().getPlugin<services::UserService>();
            auto maybeUser   = co_await userService->getUserById(id);
            if (!maybeUser)
                co_return makeJsonError("User not found", drogon::k404NotFound);

            co_return drogon::HttpResponse::newHttpJsonResponse(maybeUser->toJson());
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "getOne exception: " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto UserController::deleteOne(drogon::HttpRequestPtr req,
                                   int id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto userService = drogon::app().getPlugin<services::UserService>();
            auto ok          = co_await userService->deleteUser(id);
            if (!ok)
                co_return makeJsonError("User not found or delete failed",
                                        drogon::k500InternalServerError);

            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k204NoContent);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[USER CONTROLLER] Error (deleteOne): " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

    auto UserController::updateRole(drogon::HttpRequestPtr req,
                                    int id) -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto json = req->getJsonObject();
            if (!json || !json->isMember("role"))
                co_return makeJsonError("Missing 'role' in request body", drogon::k400BadRequest);

            std::string newRole = (*json)["role"].asString();
            if (newRole.empty())
                co_return makeJsonError("Empty role", drogon::k400BadRequest);

            if (newRole != "admin" && newRole != "user")
                co_return makeJsonError("Invalid role", drogon::k400BadRequest);

            auto userService = drogon::app().getPlugin<services::UserService>();
            auto ok          = co_await userService->updateUserRole(id, newRole);
            if (!ok)
                co_return makeJsonError("Failed to update role (user not found?)",
                                        drogon::k500InternalServerError);

            co_return makeJsonMessage("Role updated", drogon::k200OK);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "updateRole exception: " << e.what();
            co_return makeJsonError(e.what(), drogon::k500InternalServerError);
        }
    }

}  // namespace controllers