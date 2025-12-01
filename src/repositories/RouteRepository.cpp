#include "repositories/RouteRepository.h"

using namespace drogon::orm;
using namespace repositories;

RouteRepository::RouteRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

auto RouteRepository::createRoute(int userId, int preferenceId, const std::string& name,
                                  const std::string& startPointWkt, const std::string& routeLineWkt,
                                  double distanceKm,
                                  int    mapSourceId) -> drogon::Task<std::optional<models::Routes>>
{
    CoroMapper<models::Routes> mapper(dbClient_);
    try
    {
        models::Routes route;
        route.setUserId(userId);
        route.setPreferenceId(preferenceId);
        route.setName(name);

        // Преобразуем WKT в PostGIS geometry через SQL
        auto startRes =
            co_await dbClient_->execSqlCoro("SELECT ST_GeomFromText($1, 4326)", startPointWkt);
        if (!startRes.empty())
            route.setStartPoint(startRes[0][0].as<std::string>());

        auto lineRes =
            co_await dbClient_->execSqlCoro("SELECT ST_GeomFromText($1, 4326)", routeLineWkt);
        if (!lineRes.empty())
            route.setRouteLine(lineRes[0][0].as<std::string>());

        route.setDistanceKm(std::to_string(distanceKm));
        route.setMapSourceId(mapSourceId);

        auto inserted = co_await mapper.insert(route);
        co_return inserted;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to create route: " << e.base().what();
        co_return std::nullopt;
    }
}

auto RouteRepository::getRouteById(int id) -> drogon::Task<std::optional<models::Routes>>
{
    CoroMapper<models::Routes> mapper(dbClient_);
    try
    {
        auto route = co_await mapper.findByPrimaryKey(id);
        co_return route;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to get route by ID: " << e.base().what();
        co_return std::nullopt;
    }
}

auto RouteRepository::getRoutesByUserId(int userId) -> drogon::Task<std::vector<models::Routes>>
{
    CoroMapper<models::Routes> mapper(dbClient_);
    try
    {
        auto routes = co_await mapper.findBy(Criteria("user_id", CompareOperator::EQ, userId));
        co_return routes;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to get routes by user ID: " << e.base().what();
        co_return {};
    }
}

auto RouteRepository::getAllRoutes() -> drogon::Task<std::vector<models::Routes>>
{
    CoroMapper<models::Routes> mapper(dbClient_);
    try
    {
        auto routes = co_await mapper.findAll();
        co_return routes;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to get all routes: " << e.base().what();
        co_return {};
    }
}

auto RouteRepository::updateRoute(const models::Routes& route) -> drogon::Task<bool>
{
    CoroMapper<models::Routes> mapper(dbClient_);
    try
    {
        co_await mapper.update(route);
        co_return true;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to update route: " << e.base().what();
        co_return false;
    }
}

auto RouteRepository::deleteRoute(int id) -> drogon::Task<bool>
{
    CoroMapper<models::Routes> mapper(dbClient_);
    try
    {
        auto count = co_await mapper.deleteByPrimaryKey(id);
        co_return count > 0;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to delete route: " << e.base().what();
        co_return false;
    }
}
