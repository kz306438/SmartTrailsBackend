cmake_minimum_required(VERSION 3.21)

get_filename_component(PROJECT_NAME_RAW ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_NAME_SANITIZED ${PROJECT_NAME_RAW})
project(${PROJECT_NAME_SANITIZED} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Drogon REQUIRED)
find_package(GTest REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(argon2 REQUIRED)

set(SOURCES
    src/main.cpp

    src/controllers/AuthController.cpp
    src/controllers/UserController.cpp

    src/filters/AuthenticationFilter.cpp
    src/filters/AdminFilter.cpp

    src/models/MapSources.cpp
    src/models/Poi.cpp
    src/models/PoiTypes.cpp
    src/models/RoutePoi.cpp
    src/models/Routes.cpp
    src/models/SpatialRefSys.cpp
    src/models/UserPreferences.cpp
    src/models/Users.cpp

    src/repositories/MapSourceRepository.cpp
    src/repositories/PoiRepository.cpp
    src/repositories/PoiTypeRepository.cpp
    src/repositories/RoutePoiRepository.cpp
    src/repositories/RouteRepository.cpp
    src/repositories/UserPreferencesRepository.cpp
    src/repositories/UserRepository.cpp
    
    src/services/JwtService.cpp
    src/services/AuthService.cpp
    src/services/UserService.cpp
)

# set(HEADERS
#     include/controllers/AuthController.h
#     include/controllers/UserController.h
#     include/filters/JwtFilter.h
#     include/models/User.h
#     include/services/JwtService.h
#     include/utils/HashUtils.h
# )

add_executable(${PROJECT_NAME_SANITIZED} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME_SANITIZED} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME_SANITIZED}
    PRIVATE
        Drogon::Drogon
        GTest::gtest
        jwt-cpp::jwt-cpp
        argon2::argon2
)
