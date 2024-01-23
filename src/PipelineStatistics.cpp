#include "Canta/PipelineStatistics.h"
#include <Canta/Device.h>

canta::PipelineStatistics::~PipelineStatistics() {
    if (!_device)
        return;
    _device->destroyPipelineStatistics(_queryPoolIndex, _index);
}

canta::PipelineStatistics::PipelineStatistics(canta::PipelineStatistics &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_queryPoolIndex, rhs._queryPoolIndex);
    std::swap(_index, rhs._index);
    std::swap(_queryCount, rhs._queryCount);
    std::swap(_value, rhs._value);
}

auto canta::PipelineStatistics::operator=(canta::PipelineStatistics &&rhs) noexcept -> PipelineStatistics & {
    std::swap(_device, rhs._device);
    std::swap(_queryPoolIndex, rhs._queryPoolIndex);
    std::swap(_index, rhs._index);
    std::swap(_queryCount, rhs._queryCount);
    std::swap(_value, rhs._value);
    return *this;
}

void canta::PipelineStatistics::begin(canta::CommandBuffer &commandBuffer) {
    auto pool = _device->pipelineStatisticsPools()[_queryPoolIndex];
    vkCmdResetQueryPool(commandBuffer.buffer(), pool, _index * _queryCount, _queryCount);
    vkCmdBeginQuery(commandBuffer.buffer(), pool, _index * _queryCount, 0);
}

void canta::PipelineStatistics::end(canta::CommandBuffer &commandBuffer) {
    auto pool = _device->pipelineStatisticsPools()[_queryPoolIndex];
    vkCmdEndQuery(commandBuffer.buffer(), pool, _index * _queryCount);
}

auto canta::PipelineStatistics::result() -> std::expected<Stats, Error> {
    u64 _stats[_queryCount];
    auto pool = _device->pipelineStatisticsPools()[_queryPoolIndex];
    vkGetQueryPoolResults(_device->logicalDevice(), pool, _index * _queryCount, 1, _queryCount * sizeof(u64), _stats, sizeof(u64), VK_QUERY_RESULT_64_BIT);
    return Stats {
        _stats[0],
        _stats[1],
        _stats[2],
        _stats[3],
        _stats[4],
        _stats[5],
        _stats[6],
        _stats[7],
        _stats[8],
        _stats[9],
        _stats[10],
    };
}