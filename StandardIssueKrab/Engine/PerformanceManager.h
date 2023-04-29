#pragma once

class PerformanceManager
{
    private:
        std::chrono::high_resolution_clock::time_point timer;
        double prevVal;
    public:
        PerformanceManager() = default;
        ~PerformanceManager() = default;
        void Init();
        double GetCurrentValue();
};