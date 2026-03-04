#ifndef CANTA_UI_H
#define CANTA_UI_H

#include <Canta/Device.h>

namespace canta {

    auto drawPipelineStats(const PipelineStatistics::Stats& stats, std::string_view name = {}) -> bool;

    auto drawResourceStats(const Device::ResourceStats& stats, std::string_view name = {}) -> bool;

    auto drawMemoryUsage(const Device::MemoryUsage& usage, std::string_view name = {}) -> bool;

}

#endif //CANTA_UI_H