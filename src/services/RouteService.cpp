#include "services/RouteService.h"

#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

#include <iomanip>
#include <random>
#include <sstream>

#include "services/MapSourceService.h"
#include "services/UserPreferencesService.h"

namespace services
{
    static const std::string OSRM_URL = "http://osrm:5000";

    auto RouteService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        routeRepo_    = std::make_unique<repositories::RouteRepository>(dbClient);
        poiRepo_      = std::make_unique<repositories::PoiRepository>(dbClient);
        routePoiRepo_ = std::make_unique<repositories::RoutePoiRepository>(dbClient);
        osrmClient_   = drogon::HttpClient::newHttpClient(OSRM_URL);

        LOG_INFO << "[ROUTE SERVICE] Plugin started";
    }

    auto RouteService::shutdown() -> void
    {
        routeRepo_.reset();
        poiRepo_.reset();
        routePoiRepo_.reset();
        osrmClient_.reset();
        LOG_INFO << "[ROUTE SERVICE] Plugin stopped";
    }

    auto RouteService::createRoute(const dto::RequestRouteDto& dto)
        -> drogon::Task<std::optional<repositories::models::Routes>>
    {
        try
        {
            // Start point parsing
            auto startPointOpt = parseStartPoint(dto.start_point);
            if (!startPointOpt)
            {
                LOG_ERROR << "Invalid start point format: " << dto.start_point;
                co_return std::nullopt;
            }
            const auto startPoint = *startPointOpt;  // {lon, lat}

            // POI search
            const double radiusMeters = dto.distance * 1000 / 2.0;

            // Get models of POI
            // findTargetPois принимает (types, lat, lon, radius)
            // startPoint.second = lat, startPoint.first = lon
            auto selectedPois = co_await findTargetPois(dto.poi_types, startPoint.second,
                                                        startPoint.first, radiusMeters);

            if (selectedPois.empty())
            {
                LOG_WARN << "No suitable POIs found for route generation.";
                co_return std::nullopt;
            }

            // Assembling sequence of points (Start -> POI... -> Start)
            auto keyPoints = buildPathPoints(startPoint, selectedPois);

            // Request geometry of route by OSRM
            auto routeGeometry = co_await fetchOsrmPathGeometry(keyPoints);

            if (routeGeometry.empty())
            {
                LOG_ERROR << "Failed to construct OSRM geometry.";
                co_return std::nullopt;
            }

            std::string wktLine = convertToWktLineString(routeGeometry);

            co_return co_await saveRouteToDb(dto, startPoint, wktLine, selectedPois);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "RouteService::createRoute unhandled exception: " << e.what();
            co_return std::nullopt;
        }
    }

    auto RouteService::parseStartPoint(const std::string& pointStr) const -> std::optional<GeoPoint>
    {
        double lat   = 0.0;
        double lon   = 0.0;
        char   comma = 0;

        std::stringstream ss(pointStr);
        if (ss >> lon >> comma >> lat)
        {
            return std::make_pair(lon, lat);
        }
        return std::nullopt;
    }

    auto RouteService::findTargetPois(const std::vector<int>& types, double lat, double lon,
                                      double radiusMeters)
        -> drogon::Task<std::vector<repositories::models::Poi>>
    {
        std::vector<repositories::models::Poi> result;
        result.reserve(types.size());

        std::random_device rd;
        std::mt19937       gen(rd());

        for (int typeId : types)
        {
            auto pois = co_await poiRepo_->getPoisInRadius(typeId, lat, lon, radiusMeters);

            if (pois.empty())
            {
                LOG_WARN << "No POIs found for type=" << typeId << " in radius=" << radiusMeters;
                continue;
            }
            std::uniform_int_distribution<size_t> dis(0, pois.size() - 1);

            size_t randomIndex = dis(gen);
            result.push_back(pois[randomIndex]);
        }

        co_return result;
    }

    auto RouteService::buildPathPoints(GeoPoint                                      start,
                                       const std::vector<repositories::models::Poi>& pois) const
        -> std::vector<GeoPoint>
    {
        std::vector<GeoPoint> path;
        path.reserve(pois.size() + 2);

        path.push_back(start);

        // Extract coordinates from model
        for (const auto& poi : pois)
        {
            auto [lon, lat] = poi.getLonLat();
            path.emplace_back(lon, lat);
        }

        path.push_back(start);

        return path;
    }

    auto RouteService::fetchOsrmPathGeometry(const std::vector<GeoPoint>& pathPoints)
        -> drogon::Task<std::vector<GeoPoint>>
    {
        std::vector<GeoPoint> fullRouteCoords;

        for (size_t i = 0; i + 1 < pathPoints.size(); ++i)
        {
            auto [lon1, lat1] = pathPoints[i];
            auto [lon2, lat2] = pathPoints[i + 1];

            std::ostringstream pathStream;
            pathStream << "/route/v1/walking/" << lon1 << "," << lat1 << ";" << lon2 << "," << lat2;

            auto req = drogon::HttpRequest::newHttpRequest();
            req->setPath(pathStream.str());
            req->setParameter("geometries", "geojson");
            req->setParameter("overview", "full");

            auto resp = co_await osrmClient_->sendRequestCoro(req);

            if (!resp || resp->getStatusCode() != 200)
            {
                LOG_ERROR << "OSRM request failed";
                co_return std::vector<GeoPoint>{};
            }

            try
            {
                auto json = resp->getJsonObject();
                if (!json || !json->isMember("routes") || (*json)["routes"].empty())
                    co_return std::vector<GeoPoint>{};

                // GeoJSON coordinates массив: [lon, lat]
                const auto& coords = (*json)["routes"][0]["geometry"]["coordinates"];
                for (const auto& c : coords)
                    fullRouteCoords.emplace_back(c[0].asDouble(), c[1].asDouble());
            }
            catch (const std::exception& e)
            {
                LOG_ERROR << "Error parsing OSRM JSON: " << e.what();
                co_return std::vector<GeoPoint>{};
            }
        }
        co_return fullRouteCoords;
    }

    auto
    RouteService::convertToWktLineString(const std::vector<GeoPoint>& coords) const -> std::string
    {
        std::ostringstream wkt;
        wkt << "LINESTRING(";
        for (size_t i = 0; i < coords.size(); ++i)
        {
            // PostGIS WKT формат: "lon lat" (X Y)
            wkt << std::fixed << std::setprecision(6) << coords[i].first << " " << coords[i].second;
            if (i + 1 < coords.size())
                wkt << ",";
        }
        wkt << ")";
        return wkt.str();
    }

    auto RouteService::convertToWktPoint(GeoPoint point) const -> std::string
    {
        std::ostringstream wkt;
        wkt << std::fixed << std::setprecision(6) << "POINT(" << point.first << " " << point.second
            << ")";
        return wkt.str();
    }

    auto RouteService::saveRouteToDb(const dto::RequestRouteDto& dto, GeoPoint startPoint,
                                     const std::string&                            wktLineString,
                                     const std::vector<repositories::models::Poi>& selectedPois)
        -> drogon::Task<std::optional<repositories::models::Routes>>
    {
        auto userPrefService  = drogon::app().getPlugin<UserPreferencesService>();
        auto mapSourceService = drogon::app().getPlugin<MapSourceService>();

        if (!userPrefService || !mapSourceService)
        {
            LOG_ERROR << "Dependent services not found";
            co_return std::nullopt;
        }

        auto prefOpt =
            co_await userPrefService->createPreferecnes(dto.user_id, dto.distance, dto.poi_types);
        if (!prefOpt)
        {
            LOG_ERROR << "Failed to create user preferences";
            co_return std::nullopt;
        }

        auto mapSrcOpt = co_await mapSourceService->getActiveMapSource();
        if (!mapSrcOpt)
        {
            LOG_ERROR << "Failed to get active map source";
            co_return std::nullopt;
        }

        const int userId       = dto.user_id;
        const int preferenceId = *prefOpt.value().getId();
        const int mapSourceId  = *mapSrcOpt.value().getId();

        std::string wktPointStr     = convertToWktPoint(startPoint);
        double      totalDistanceKm = dto.distance;

        // Save route
        auto savedRoute =
            co_await routeRepo_->createRoute(userId, preferenceId, dto.name, wktPointStr,
                                             wktLineString, totalDistanceKm, mapSourceId);

        if (!savedRoute)
        {
            LOG_ERROR << "Failed to save route to DB";
            co_return std::nullopt;
        }

        // Save relations into route_poi
        const int routeId = *savedRoute->getId();

        for (const auto& poi : selectedPois)
        {
            if (poi.getId())
            {
                bool added = co_await routePoiRepo_->addPoiToRoute(routeId, *poi.getId());
                if (!added)
                {
                    LOG_WARN << "Failed to link POI id=" << *poi.getId()
                             << " to route id=" << routeId;
                }
            }
        }

        LOG_INFO << "Route created successfully for user " << userId << " with "
                 << selectedPois.size() << " POIs";
        co_return savedRoute;
    }

}  // namespace services