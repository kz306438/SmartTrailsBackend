#pragma once

#include <string>
#include <vector>

namespace dto
{

    struct RequestRouteDto
    {
        int              user_id;
        std::string      name;
        std::string      start_point;
        float            distance;
        std::vector<int> poi_types;
    };

}  // namespace dto