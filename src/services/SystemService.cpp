#include "services/SystemService.h"

#include <drogon/drogon.h>

#include <fstream>
#include <sstream>
#include <string>

namespace services
{

    void SystemService::initAndStart(const Json::Value& config)
    {
        auto dbClient = drogon::app().getDbClient();
        userRepo_     = std::make_unique<repositories::UserRepository>(dbClient);
        LOG_INFO << "[SYSTEM SERVICE] Plugin started";
    }

    void SystemService::shutdown()
    {
        LOG_INFO << "[SYSTEM SERVICE] Plugin stopped";
    }

    auto SystemService::readMemInfo() -> std::pair<double, double>
    {
        std::ifstream file("/proc/meminfo");
        std::string   line;
        double        total = 0, available = 0;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string       key;
            double            value;
            std::string       unit;
            ss >> key >> value >> unit;

            if (key == "MemTotal:")
                total = value;
            else if (key == "MemAvailable:")
                available = value;

            if (total > 0 && available > 0)
                break;
        }
        return {total - available, total};
    }

    drogon::Task<SystemService::SystemMetrics> SystemService::getMetrics()
    {
        SystemMetrics metrics{0.0, 0.0, 0};

        // 1. RAM
        auto [usedMem, totalMem] = readMemInfo();
        if (totalMem > 0)
        {
            metrics.memoryUsagePercent = (usedMem / totalMem) * 100.0;
        }

        // 2. CPU
        std::ifstream file("/proc/stat");
        std::string   line;
        if (std::getline(file, line))
        {
            std::stringstream  ss(line);
            std::string        cpuLabel;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
            ss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

            unsigned long long totalIdle    = idle + iowait;
            unsigned long long totalNonIdle = user + nice + system + irq + softirq + steal;
            unsigned long long total        = totalIdle + totalNonIdle;

            unsigned long long totalDiff = total - prevTotal_;
            unsigned long long idleDiff  = totalIdle - prevIdle_;

            if (totalDiff > 0)
            {
                metrics.cpuUsagePercent = (double)(totalDiff - idleDiff) / totalDiff * 100.0;
            }

            prevTotal_ = total;
            prevIdle_  = totalIdle;
        }

        // 3. Users (Count total from DB)
        auto users          = co_await userRepo_->getAllUsers();
        metrics.activeUsers = (int)users.size();

        co_return metrics;
    }
}  // namespace services