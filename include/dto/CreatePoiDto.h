#pragma once
#include <optional>
#include <string>

namespace dto
{

    struct CreatePoiDto
    {
        std::optional<std::string> name;
        std::optional<std::string> city;
        int                        typeId;
        std::string                typeName;
        double                     lon = 0.0;
        double                     lat = 0.0;
        std::optional<std::string> description;
        int                        mapSourceId;

        std::string coordinatesWkt() const
        {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "POINT(%.8f %.8f)", lon, lat);
            return std::string(buf);
        }
    };

}  // namespace dto
