#include "stdafx.h"
#include "PerformanceManager.h"

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void PerformanceManager::Init() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);

    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    timer = std::chrono::high_resolution_clock::now();
    prevVal = 0.0;
}

double PerformanceManager::GetCurrentValue() {

    long long count = 0;
    count = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timer).count();
    if (count < 1000) return prevVal;
    timer = std::chrono::high_resolution_clock::now();

    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    prevVal = counterVal.doubleValue;
    return counterVal.doubleValue;
}