#ifndef CANTA_COMMANDBUFFER_H
#define CANTA_COMMANDBUFFER_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Semaphore.h>
#include <span>
#include <Canta/ResourceList.h>
#include <Ende/math/Vec.h>

namespace canta {

    class CommandPool;
    class Device;

    class Pipeline;
    using PipelineHandle = Handle<Pipeline, ResourceList<Pipeline>>;

    struct Attachment {
        VkImageView imageView = VK_NULL_HANDLE;
        ImageLayout imageLayout = ImageLayout::UNDEFINED;
    };

    struct RenderingInfo {
        ende::math::Vec<2, u32> size = { 0, 0 };
        ende::math::Vec<2, i32> offset = { 0, 0 };
        std::span<Attachment> colourAttachments = {};
        VkImageView depthAttachment = VK_NULL_HANDLE;
    };

    struct ImageBarrier {
        VkImage image = VK_NULL_HANDLE;
        PipelineStage srcStage = PipelineStage::TOP;
        PipelineStage dstStage = PipelineStage::BOTTOM;
        Access srcAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        Access dstAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        ImageLayout srcLayout = ImageLayout::UNDEFINED;
        ImageLayout dstLayout = ImageLayout::UNDEFINED;
    };

    class CommandBuffer {
    public:

        CommandBuffer(CommandBuffer&& rhs) noexcept;
        auto operator=(CommandBuffer&& rhs) noexcept -> CommandBuffer&;

        auto buffer() const -> VkCommandBuffer { return _buffer; }

        auto submit(std::span<Semaphore::Pair> waitSemaphores, std::span<Semaphore::Pair> signalSemaphores, VkFence fence = VK_NULL_HANDLE) -> std::expected<bool, Error>;

        auto begin() -> bool;
        auto end() -> bool;

        void beginRendering(RenderingInfo info);
        void endRendering();

        void setViewport(const ende::math::Vec<2, f32>& size, const ende::math::Vec<2, f32>& offset = { 0, 0 }, bool setScissor = true);
        void setScissor(const ende::math::Vec<2, u32>& size, const ende::math::Vec<2, i32>& offset = { 0, 0 });

        void bindPipeline(PipelineHandle pipeline);

        void draw(u32 count, u32 instanceCount = 1, u32 first = 0, u32 firstInstance = 0);

        void barrier(ImageBarrier barrier);

    private:
        friend CommandPool;

        CommandBuffer() = default;

        Device* _device = nullptr;
        VkCommandBuffer _buffer = VK_NULL_HANDLE;
        QueueType _queueType = QueueType::NONE;

        PipelineHandle _currentPipeline = {};

    };

}

#endif //CANTA_COMMANDBUFFER_H
