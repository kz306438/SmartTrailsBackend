#pragma once
#include <string>
#include <vector>

#include "dto/CreatePoiDto.h"

namespace utils
{
    auto parseGeoJsonToCreateDtos(const std::string& geojsonStr) -> std::vector<dto::CreatePoiDto>;

    auto mapToTypeName(const std::string& key, const std::string& value) -> std::string;

}  // namespace utils
