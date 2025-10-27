#include <drogon/drogon.h>

#include "services/JwtService.h"

int main()
{
    drogon::app().loadConfigFile("config.json").setLogLevel(trantor::Logger::kTrace).run();
}