#include "services/RouteService.h"

#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

namespace services
{
    auto RouteService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        routeRepo_    = std::make_unique<repositories::RouteRepository>(dbClient);
        poiRepo_      = std::make_unique<repositories::PoiRepository>(dbClient);
        LOG_INFO << "[ROUTE SERVICE] Plugin started";
    }

    auto RouteService::shutdown() -> void
    {
        routeRepo_.reset();
        poiRepo_.reset();
        LOG_INFO << "[ROUTE SERVICE] Plugin stopped";
    }

    auto RouteService::createRoute(const dto::RequestRouteDto& dto)
        -> drogon::Task<std::optional<repositories::models::Routes>>
    {
        try
        {
            // -------------------------
            // 1. Парсим стартовую точку "lon,lat"
            // -------------------------
            double startLon = 0, startLat = 0;
            {
                std::stringstream ss(dto.start_point);
                char              comma = 0;
                ss >> startLon >> comma >> startLat;
            }

            // -------------------------
            // 2. Получаем POI и парсим WKT
            // -------------------------
            std::vector<std::pair<double, double>> selected;  // lon, lat
            const double                           radius = dto.distance / 2.0;

            for (int typeId : dto.poi_types)
            {
                auto pois = co_await poiRepo_->getPoisInRadius(typeId, startLon, startLat, radius);
                if (pois.empty())
                {
                    LOG_WARN << "No POIs for type=" << typeId;
                    continue;
                }

                const auto& poi = pois.front();
                std::string wkt = poi.getValueOfCoordinates();

                // --- Парсинг POINT(lon lat) ---
                double lon = 0, lat = 0;
                if (wkt.rfind("POINT", 0) == 0)
                {
                    auto L = wkt.find('(');
                    auto R = wkt.find(')');
                    if (L != std::string::npos && R != std::string::npos && R > L)
                    {
                        std::string       inside = wkt.substr(L + 1, R - L - 1);
                        std::stringstream ss(inside);
                        ss >> lon >> lat;
                    }
                }

                selected.emplace_back(lon, lat);
            }

            if (selected.empty())
            {
                LOG_WARN << "No POIs selected";
                co_return std::nullopt;
            }

            // -------------------------
            // 3. Формируем последовательность точек маршрута
            // -------------------------
            std::vector<std::pair<double, double>> pathPoints;
            pathPoints.emplace_back(startLon, startLat);
            for (auto& p : selected)
                pathPoints.push_back(p);
            pathPoints.emplace_back(startLon, startLat);  // возврат

            // -------------------------
            // 4. Запрашиваем OSRM (частями)
            // -------------------------
            auto http = drogon::HttpClient::newHttpClient("http://127.0.0.1:5000");
            std::vector<std::pair<double, double>> routeCoords;

            for (size_t i = 0; i + 1 < pathPoints.size(); ++i)
            {
                auto [lon1, lat1] = pathPoints[i];
                auto [lon2, lat2] = pathPoints[i + 1];

                std::ostringstream p;
                p << "/route/v1/driving/" << lon1 << "," << lat1 << ";" << lon2 << "," << lat2;

                auto req = drogon::HttpRequest::newHttpRequest();
                req->setPath(p.str());
                req->setParameter("geometries", "geojson");
                req->setParameter("overview", "full");

                auto resp = co_await http->sendRequestCoro(req);
                if (!resp || resp->getStatusCode() != 200)
                {
                    LOG_ERROR << "OSRM request failed";
                    co_return std::nullopt;
                }

                auto        json   = resp->getJsonObject();
                const auto& coords = (*json)["routes"][0]["geometry"]["coordinates"];

                for (const auto& c : coords)
                    routeCoords.emplace_back(c[0].asDouble(), c[1].asDouble());
            }

            if (routeCoords.empty())
            {
                LOG_ERROR << "No OSRM route points";
                co_return std::nullopt;
            }

            // -------------------------
            // 5. Формируем итоговый LineString в WKT
            // -------------------------
            std::ostringstream wktLine;
            wktLine << "LINESTRING(";
            for (size_t i = 0; i < routeCoords.size(); ++i)
            {
                auto [lon, lat] = routeCoords[i];
                wktLine << lon << " " << lat;
                if (i + 1 < routeCoords.size())
                    wktLine << ",";
            }
            wktLine << ")";

            // -------------------------
            // 6. Формируем стартовую точку для таблицы (Point)
            // -------------------------
            std::ostringstream wktPoint;
            wktPoint << "POINT(" << startLon << " " << startLat << ")";

            // -------------------------
            // 7. Сохраняем в БД
            // -------------------------
            const int userId       = dto.user_id;
            const int preferenceId = 0;  // если у вас нет в dto — ставим NULL
            const int mapSourceId = 1;  // при необходимости подставьте реальный id

            double totalDistanceKm = dto.distance;  // пока используем исходную

            auto saved = co_await routeRepo_->createRoute(userId, preferenceId, dto.name,
                                                          wktPoint.str(),  // start_point
                                                          wktLine.str(),   // route_line
                                                          totalDistanceKm, mapSourceId);

            co_return saved;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "RouteService::createRoute error: " << e.what();
            co_return std::nullopt;
        }
    }
}  // namespace services
