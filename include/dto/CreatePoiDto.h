#pragma once
#include <optional>
#include <string>

namespace dto
{

    struct CreatePoiDto
    {
      public:
        std::optional<std::string> name;
        std::optional<std::string> city;
        int                        typeId;
        std::string                typeName;
        std::string                coordinates;
        std::optional<std::string> description;
        int                        mapSourceId;

      public:
        std::string coordinatesWkt() const;
    };

}  // namespace dto
