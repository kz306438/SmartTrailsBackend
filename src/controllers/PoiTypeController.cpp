#include "controllers/PoiTypeController.h"

#include "services/PoiTypeService.h"
#include "utils/JsonResponseUtil.h"

namespace controllers
{

    auto PoiTypeController::getPoiTypes(drogon::HttpRequestPtr req)
        -> drogon::Task<drogon::HttpResponsePtr>
    {
        try
        {
            auto poiTypeService = drogon::app().getPlugin<services::PoiTypeService>();
            auto poiTypes       = co_await poiTypeService->getPoiTypes();

            if (poiTypes.empty())
                co_return utils::makeJsonMessage("POI types not found", drogon::k404NotFound);

            Json::Value arr(Json::arrayValue);

            for (const auto& poiType : poiTypes)
            {
                arr.append(poiType.toJson());
            }

            auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
            co_return resp;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "[POI-TYPE CONTROLLER] Error (getPoiTypes): " << e.what();
            co_return utils::makeJsonError("Internal Error", drogon::k500InternalServerError);
        }
    }

}  // namespace controllers