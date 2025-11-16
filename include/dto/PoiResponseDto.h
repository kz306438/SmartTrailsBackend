#pragma once

#include <optional>
#include <string>

namespace dto
{

    struct PoiResponseDto
    {
        int                        id;
        std::string                name;
        std::optional<std::string> city;
        double                     lon = 0.0;
        double                     lat = 0.0;
        std::optional<std::string> description;
        std::optional<int>         typeId;
        std::optional<int>         mapSourceId;
    };

}  // namespace dto
