// #pragma once

// #include <drogon/HttpController.h>

// namespace controllers
// {

// // CREATE TABLE user_preferences (
// //     id                  SERIAL PRIMARY KEY,
// //     user_id             INT REFERENCES users(id) ON DELETE CASCADE,
// //     preferred_distance_km NUMERIC(6,2),
// //     poi_type_ids        INT[],
// //     updated_at          TIMESTAMPTZ DEFAULT NOW()
// // );

//     class UserPreferencesController : public drogon::HttpController<UserPreferencesController>
//     {
//       public:
//         METHOD_LIST_BEGIN

//         ADD_METHOD_TO(UserPreferencesController::createUserPreferences, "/api/user-preferences",
//         drogon::Get,
//                       "filters::AuthenticationFilter");
//         ADD_METHOD_TO(UserPreferencesController::updateMe, "/api/users/me", drogon::Put,
//                       "filters::AuthenticationFilter");
//         ADD_METHOD_TO(UserPreferencesController::deleteMe, "/api/users/me", drogon::Delete,
//                       "filters::AuthenticationFilter");

//         ADD_METHOD_TO(UserPreferencesController::getAll, "/api/users", drogon::Get,
//                       "filters::AuthenticationFilter", "filters::AdminFilter");
//         ADD_METHOD_TO(UserPreferencesController::getOne, "/api/users/{id}", drogon::Get,
//                       "filters::AuthenticationFilter", "filters::AdminFilter");
//         ADD_METHOD_TO(UserPreferencesController::deleteOne, "/api/users/{id}", drogon::Delete,
//                       "filters::AuthenticationFilter", "filters::AdminFilter");
//         ADD_METHOD_TO(UserPreferencesController::updateRole, "/api/users/{id}/role",
//         drogon::Patch,
//                       "filters::AuthenticationFilter", "filters::AdminFilter");

//         METHOD_LIST_END

//       public:
//         auto getMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
//         auto updateMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
//         auto deleteMe(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;

//         auto getAll(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr>;
//         auto getOne(drogon::HttpRequestPtr req, int id) -> drogon::Task<drogon::HttpResponsePtr>;
//         auto deleteOne(drogon::HttpRequestPtr req, int id) ->
//         drogon::Task<drogon::HttpResponsePtr>; auto updateRole(drogon::HttpRequestPtr req,
//                         int                    id) -> drogon::Task<drogon::HttpResponsePtr>;
//     };

// }  // namespace controllers
