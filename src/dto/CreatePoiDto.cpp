#include "dto/CreatePoiDto.h"

#include <stdexcept>

namespace dto
{

    std::string CreatePoiDto::coordinatesWkt() const
    {
        // coordinates expected like "x,y"
        auto pos = coordinates.find(',');
        if (pos == std::string::npos)
            throw std::runtime_error("Invalid coordinates format. Expected 'x,y'.");

        std::string x = coordinates.substr(0, pos);
        std::string y = coordinates.substr(pos + 1);

        // Remove spaces
        x.erase(0, x.find_first_not_of(" \t"));
        x.erase(x.find_last_not_of(" \t") + 1);
        y.erase(0, y.find_first_not_of(" \t"));
        y.erase(y.find_last_not_of(" \t") + 1);

        return "POINT(" + x + " " + y + ")";
    }

}  // namespace dto