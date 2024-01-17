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
    class Image;
    class Buffer;
    class Sampler;

    using PipelineHandle = Handle<Pipeline, ResourceList<Pipeline>>;
    using ImageHandle = Handle<Image, ResourceList<Image>>;
    using BufferHandle = Handle<Buffer, ResourceList<Buffer>>;
    using SamplerHandle = Handle<Sampler, ResourceList<Sampler>>;

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
        ImageHandle image = {};
        PipelineStage srcStage = PipelineStage::TOP;
        PipelineStage dstStage = PipelineStage::BOTTOM;
        Access srcAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        Access dstAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        ImageLayout srcLayout = ImageLayout::UNDEFINED;
        ImageLayout dstLayout = ImageLayout::UNDEFINED;
        u32 srcQueue = -1;
        u32 dstQueue = -1;
    };

    struct BufferBarrier {
        BufferHandle buffer = {};
        PipelineStage srcStage = PipelineStage::TOP;
        PipelineStage dstStage = PipelineStage::BOTTOM;
        Access srcAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        Access dstAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        u32 offset = 0;
        u32 size = 0;
        u32 srcQueue = -1;
        u32 dstQueue = -1;
    };

    struct MemoryBarrier {
        PipelineStage srcStage = PipelineStage::TOP;
        PipelineStage dstStage = PipelineStage::BOTTOM;
        Access srcAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
        Access dstAccess = Access::MEMORY_READ | Access::MEMORY_WRITE;
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

        void pushConstants(ShaderStage stage, std::span<const u8> data, u32 offset = 0);
        template <typename T>
        void pushConstants(ShaderStage stage, const T& data, u32 offset = 0) {
            pushConstants(stage, std::span<const u8>(reinterpret_cast<const u8*>(&data), sizeof(T)), offset);
        }

        void draw(u32 count, u32 instanceCount = 1, u32 first = 0, u32 firstInstance = 0);

        void dispatchWorkgroups(u32 x = 1, u32 y = 1, u32 z = 1);
        void dispatchThreads(u32 x = 1, u32 y = 1, u32 z = 0);

        struct BlitInfo {
            ImageHandle src = {};
            u32 srcMip = 0;
            u32 srcLayer = 0;
            u32 srcLayerCount = 0;
            ImageHandle dst = {};
            u32 dstMip = 0;
            u32 dstLayer = 0;
            u32 dstLayerCount = 0;
            ImageLayout srcLayout = ImageLayout::UNDEFINED;
            ImageLayout dstLayout = ImageLayout::UNDEFINED;
            Filter filter = Filter::LINEAR;
        };
        void blit(BlitInfo info);
        void clearImage(ImageHandle handle, ImageLayout layout = ImageLayout::GENERAL, const std::array<f32, 4>& clearColour = { 0, 0, 0, 1 });

        void barrier(ImageBarrier barrier);
        void barrier(BufferBarrier barrier);
        void barrier(MemoryBarrier barrier);

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
