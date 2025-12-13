#pragma once
#include <optional>
#include <string>
#include <variant>

namespace utils
{

    enum class ServiceErrorType
    {
        None,
        BadRequest,
        NotFound,
        Conflict,
        Unauthorized,
        Forbidden,
        InternalError
    };

    template <typename T> struct ServiceResult
    {
        std::optional<T> data;

        // Error metadata
        ServiceErrorType errorType = ServiceErrorType::None;
        std::string      errorMessage;
        std::string      errorField;

        static ServiceResult<T> Ok(T val)
        {
            return {std::move(val), ServiceErrorType::None, "", ""};
        }

        static ServiceResult<T> Conflict(const std::string& msg, const std::string& field = "")
        {
            return {std::nullopt, ServiceErrorType::Conflict, msg, field};
        }

        static ServiceResult<T> NotFound(const std::string& msg)
        {
            return {std::nullopt, ServiceErrorType::NotFound, msg, ""};
        }

        static ServiceResult<T> Internal(const std::string& msg)
        {
            return {std::nullopt, ServiceErrorType::InternalError, msg, ""};
        }

        static ServiceResult<T> BadRequest(const std::string& msg)
        {
            return {std::nullopt, ServiceErrorType::BadRequest, msg, ""};
        }

        static ServiceResult<T> Unauthorized(const std::string& msg)
        {
            return {std::nullopt, ServiceErrorType::Unauthorized, msg, ""};
        }

        static ServiceResult<T> Forbidden(const std::string& msg)
        {
            return {std::nullopt, ServiceErrorType::Forbidden, msg, ""};
        }

        bool isOk() const
        {
            return errorType == ServiceErrorType::None;
        }
    };

    // Void specialization
    struct ServiceVoidResult
    {
        ServiceErrorType errorType = ServiceErrorType::None;
        std::string      errorMessage;
        std::string      errorField;

        static ServiceVoidResult Ok()
        {
            return {ServiceErrorType::None, "", ""};
        }

        static ServiceVoidResult Conflict(const std::string& msg, const std::string& field = "")
        {
            return {ServiceErrorType::Conflict, msg, field};
        }

        static ServiceVoidResult NotFound(const std::string& msg)
        {
            return {ServiceErrorType::NotFound, msg, ""};
        }

        static ServiceVoidResult Internal(const std::string& msg)
        {
            return {ServiceErrorType::InternalError, msg, ""};
        }

        static ServiceVoidResult BadRequest(const std::string& msg)
        {
            return {ServiceErrorType::BadRequest, msg, ""};
        }

        static ServiceVoidResult Forbidden(const std::string& msg)
        {
            return {ServiceErrorType::Forbidden, msg, ""};
        }

        bool isOk() const
        {
            return errorType == ServiceErrorType::None;
        }
    };

}  // namespace utils