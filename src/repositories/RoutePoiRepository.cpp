#include "repositories/RoutePoiRepository.h"

using namespace drogon::orm;
using namespace repositories;

RoutePoiRepository::RoutePoiRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

auto RoutePoiRepository::addPoiToRoute(int routeId, int poiId) -> drogon::Task<bool>
{
    CoroMapper<models::RoutePoi> mapper(dbClient_);
    try
    {
        models::RoutePoi routePoi;
        routePoi.setRouteId(routeId);
        routePoi.setPoiId(poiId);

        co_await mapper.insert(routePoi);
        co_return true;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to add POI to route: " << e.base().what();
        co_return false;
    }
}

auto RoutePoiRepository::removePoiFromRoute(int routeId, int poiId) -> drogon::Task<bool>
{
    CoroMapper<models::RoutePoi> mapper(dbClient_);
    try
    {
        auto deletedCount =
            co_await mapper.deleteBy(Criteria("route_id", CompareOperator::EQ, routeId) &&
                                     Criteria("poi_id", CompareOperator::EQ, poiId));

        co_return deletedCount > 0;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to remove POI from route: " << e.base().what();
        co_return false;
    }
}

auto RoutePoiRepository::getPoisByRoute(int routeId) -> drogon::Task<std::vector<models::RoutePoi>>
{
    CoroMapper<models::RoutePoi> mapper(dbClient_);
    try
    {
        auto result = co_await mapper.findBy(Criteria("route_id", CompareOperator::EQ, routeId));
        co_return result;
    }
    catch (const DrogonDbException& e)
    {
        LOG_ERROR << "Failed to get POIs by route: " << e.base().what();
        co_return {};
    }
}
