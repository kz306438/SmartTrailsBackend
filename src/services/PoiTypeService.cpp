#include "services/PoiTypeService.h"

#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

namespace services
{
    auto PoiTypeService::initAndStart(const Json::Value& config) -> void
    {
        auto dbClient = drogon::app().getDbClient();
        repo_         = std::make_unique<repositories::PoiTypeRepository>(dbClient);
        LOG_INFO << "[POI-TYPE SERVICE] Plugin started";
    }

    auto PoiTypeService::shutdown() -> void
    {
        repo_.reset();
        LOG_INFO << "[POI-TYPE SERVICE] Plugin stopped";
    }

    auto PoiTypeService::getPoiTypes() -> drogon::Task<std::vector<repositories::models::PoiTypes>>
    {
        co_return co_await repo_->getAllPoiTypes();
    }

}  // namespace services
