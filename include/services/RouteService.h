#pragma once

#include <drogon/HttpClient.h>
#include <drogon/plugins/Plugin.h>

#include <optional>
#include <string>
#include <vector>

#include "dto/RequestRouteDto.h"
#include "repositories/PoiRepository.h"
#include "repositories/RoutePoiRepository.h"  // <--- 1. Добавили инклюд
#include "repositories/RouteRepository.h"

namespace services
{

    class RouteService : public drogon::Plugin<RouteService>
    {
      public:
        using GeoPoint = std::pair<double, double>;  // first = lon, second = lat

        auto initAndStart(const Json::Value& config) -> void override;
        auto shutdown() -> void override;

        [[nodiscard]]
        auto createRoute(const dto::RequestRouteDto& dto)
            -> drogon::Task<std::optional<repositories::models::Routes>>;

      private:
        // logic helpers
        auto parseStartPoint(const std::string& pointStr) const -> std::optional<GeoPoint>;

        // <--- 2. Изменили возвращаемый тип на вектор моделей
        auto
        findTargetPois(const std::vector<int>& types, double lat, double lon,
                       double radiusMeters) -> drogon::Task<std::vector<repositories::models::Poi>>;

        // <--- 3. Изменили входной тип на вектор моделей
        auto buildPathPoints(GeoPoint start, const std::vector<repositories::models::Poi>& pois)
            const -> std::vector<GeoPoint>;

        auto fetchOsrmPathGeometry(const std::vector<GeoPoint>& pathPoints)
            -> drogon::Task<std::vector<GeoPoint>>;

        // <--- 4. Добавили аргумент selectedPois
        auto saveRouteToDb(const dto::RequestRouteDto& dto, GeoPoint startPoint,
                           const std::string&                            wktLineString,
                           const std::vector<repositories::models::Poi>& selectedPois)
            -> drogon::Task<std::optional<repositories::models::Routes>>;

        // utils helpers
        auto convertToWktLineString(const std::vector<GeoPoint>& coords) const -> std::string;
        auto convertToWktPoint(GeoPoint point) const -> std::string;

      private:
        std::unique_ptr<repositories::RouteRepository> routeRepo_;
        std::unique_ptr<repositories::PoiRepository>   poiRepo_;
        std::unique_ptr<repositories::RoutePoiRepository>
            routePoiRepo_;  // <--- 5. Добавили репозиторий
        std::shared_ptr<drogon::HttpClient> osrmClient_;
    };

}  // namespace services