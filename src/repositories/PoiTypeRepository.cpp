#include "repositories/PoiTypeRepository.h"

using namespace drogon::orm;

namespace repositories
{

    PoiTypeRepository::PoiTypeRepository(DbClientPtr dbClient) : dbClient_(std::move(dbClient)) {}

    auto PoiTypeRepository::createPoiType(const std::string& name)
        -> drogon::Task<std::optional<models::PoiTypes>>
    {
        try
        {
            models::PoiTypes poiType;
            poiType.setName(name);

            CoroMapper<models::PoiTypes> mapper(dbClient_);
            co_await mapper.insert(poiType);

            co_return poiType;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (createPoiType): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto PoiTypeRepository::getPoiTypeById(int id) -> drogon::Task<std::optional<models::PoiTypes>>
    {
        try
        {
            CoroMapper<models::PoiTypes> mapper(dbClient_);
            auto                         poiType = co_await mapper.findByPrimaryKey(id);
            co_return poiType;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getPoiTypeById): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto PoiTypeRepository::getPoiTypeByName(const std::string& name)
        -> drogon::Task<std::optional<models::PoiTypes>>
    {
        try
        {
            CoroMapper<models::PoiTypes> mapper(dbClient_);
            auto                         poiType = co_await mapper.findOne(
                Criteria(models::PoiTypes::Cols::_name, CompareOperator::EQ, name));
            co_return poiType;
        }
        catch (const DrogonDbException& e)
        {
            LOG_WARN << "[REPOSITORY] Error (getPoiTypeByName): " << e.base().what();
            co_return std::nullopt;
        }
    }

    auto PoiTypeRepository::getAllPoiTypes() -> drogon::Task<std::vector<models::PoiTypes>>
    {
        try
        {
            CoroMapper<models::PoiTypes> mapper(dbClient_);
            auto                         poiTypes = co_await mapper.findAll();
            co_return poiTypes;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (getAllPoiTypes): " << e.base().what();
            co_return {};
        }
    }

    auto PoiTypeRepository::updatePoiType(const models::PoiTypes& poiType) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::PoiTypes> mapper(dbClient_);
            co_await mapper.update(poiType);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (updatePoiType): " << e.base().what();
            co_return false;
        }
    }

    auto PoiTypeRepository::deletePoiType(int id) -> drogon::Task<bool>
    {
        try
        {
            CoroMapper<models::PoiTypes> mapper(dbClient_);
            co_await mapper.deleteByPrimaryKey(id);
            co_return true;
        }
        catch (const DrogonDbException& e)
        {
            LOG_ERROR << "[REPOSITORY] Error (deletePoiType): " << e.base().what();
            co_return false;
        }
    }

}  // namespace repositories
