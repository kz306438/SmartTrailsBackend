#pragma once
#include <drogon/plugins/Plugin.h>

#include "repositories/UserRepository.h"

namespace services
{
    class SystemService : public drogon::Plugin<SystemService>
    {
      public:
        void initAndStart(const Json::Value& config) override;
        void shutdown() override;

        struct SystemMetrics
        {
            double cpuUsagePercent;
            double memoryUsagePercent;
            int    activeUsers;
        };

        drogon::Task<SystemMetrics> getMetrics();

      private:
        std::unique_ptr<repositories::UserRepository> userRepo_;

        // For CPU calculation (previous ticks)
        unsigned long long prevIdle_  = 0;
        unsigned long long prevTotal_ = 0;

        std::pair<double, double> readMemInfo();  // returns {used, total}
    };
}  // namespace services