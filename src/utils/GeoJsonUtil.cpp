#include "utils/GeoJsonUtil.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

namespace utils
{

    auto parseGeoJsonToCreateDtos(const std::string& geojsonStr) -> std::vector<dto::CreatePoiDto>
    {
        std::vector<dto::CreatePoiDto> out;
        if (geojsonStr.empty())
            return out;

        Json::CharReaderBuilder builder;
        Json::Value             root;
        std::string             errs;
        std::istringstream      ss(geojsonStr);

        if (!Json::parseFromStream(builder, ss, &root, &errs))
        {
            LOG_ERROR << "[GeoJsonUtil] GeoJSON parse error: " << errs;
            return out;
        }

        if (!root.isMember("features") || !root["features"].isArray())
            return out;

        for (const auto& feature : root["features"])
        {
            try
            {
                const Json::Value* geom = nullptr;
                if (feature.isMember("geometry") && feature["geometry"].isObject())
                    geom = &feature["geometry"];
                if (!geom || !geom->isMember("type") ||
                    geom->operator[]("type").asString() != "Point")
                    continue;

                const auto& coords = (*geom)["coordinates"];
                if (!coords.isArray() || coords.size() < 2)
                    continue;

                dto::CreatePoiDto dto;
                dto.lon = coords[0].asDouble();
                dto.lat = coords[1].asDouble();

                const Json::Value& props =
                    (feature.isMember("properties") && feature["properties"].isObject())
                        ? feature["properties"]
                        : Json::Value(Json::objectValue);

                auto assignIfString = [&](const char* key, std::optional<std::string>& field)
                {
                    if (props.isMember(key) && props[key].isString())
                        field = props[key].asString();
                    else
                        field.reset();
                };

                assignIfString("name", dto.name);
                assignIfString("city", dto.city);
                assignIfString("description", dto.description);

                std::string keyFound;
                std::string valueFound;
                for (const auto& k : {"amenity", "tourism", "leisure"})
                {
                    if (props.isMember(k) && props[k].isString())
                    {
                        keyFound   = k;
                        valueFound = props[k].asString();
                        break;
                    }
                }

                if (keyFound.empty())
                    continue;

                auto mapped = mapToTypeName(keyFound, valueFound);
                if (mapped.empty())
                    continue;

                dto.typeName = std::move(mapped);
                out.push_back(std::move(dto));
            }
            catch (const std::exception& e)
            {
                LOG_ERROR << "[GeoJsonUtil] Skipping feature due to exception: " << e.what();
            }
        }

        return out;
    }

    auto mapToTypeName(const std::string& key, const std::string& value) -> std::string
    {
        static const std::unordered_map<std::string, std::string> mapping = {
            {"amenity:restaurant", "restaurant"}, {"amenity:fast_food", "fast_food"},
            {"amenity:bench", "bench"},           {"tourism:museum", "tourism"},
            {"tourism:artwork", "tourism"},       {"leisure:playground", "sport"},
            {"leisure:pitch", "sport"},
        };

        if (key.empty() || value.empty())
            return std::string();

        auto it = mapping.find(key + ":" + value);
        if (it != mapping.end())
            return it->second;

        return std::string();
    }

}  // namespace utils
