#include <drogon/drogon.h>

int main()
{
    drogon::app().loadConfigFile("config.json").setLogLevel(trantor::Logger::kTrace).run();
}