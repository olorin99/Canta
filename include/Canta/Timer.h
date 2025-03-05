#ifndef CANTA_TIMER_H
#define CANTA_TIMER_H

#include <Ende/platform.h>
#include <Canta/Enums.h>
#include <expected>

namespace canta {

    class Device;
    class CommandBuffer;

    class Timer {
    public:

        Timer() = default;
        ~Timer();

        Timer(Timer&& rhs) noexcept;
        auto operator=(Timer&& rhs) noexcept -> Timer&;

        void begin(CommandBuffer& commandBuffer, PipelineStage stage = PipelineStage::TOP);
        void end(CommandBuffer& commandBuffer, PipelineStage stage = PipelineStage::BOTTOM);

        auto result() -> std::expected<u64, VulkanError>;

    private:
        friend Device;

        Device* _device = nullptr;
        u32 _queryPoolIndex = 0;
        u32 _index = 0;
        u64 _value = 0;

    };

}

#endif //CANTA_TIMER_H
