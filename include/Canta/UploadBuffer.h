#ifndef CANTA_UPLOADBUFFER_H
#define CANTA_UPLOADBUFFER_H

#include <Canta/Device.h>
#include <mutex>

namespace canta {


    class UploadBuffer {
    public:

        struct CreateInfo {
            Device* device = nullptr;
            u32 size = 0;
        };

        static auto create(CreateInfo info) -> UploadBuffer;

        UploadBuffer() = default;

        ~UploadBuffer();
        UploadBuffer(UploadBuffer&& rhs) noexcept;
        auto operator=(UploadBuffer&& rhs) noexcept -> UploadBuffer&;

        auto upload(BufferHandle dstHandle, std::span<const u8> data, u32 dstOffset = 0) -> u32;

        template <typename T>
        auto upload(BufferHandle dstHandle, const T& data, u32 dstOffset = 0) -> u32 {
            return upload(dstHandle, std::span<const u8>(reinterpret_cast<const u8*>(&data), sizeof(T)), dstOffset);
        }

        template <std::ranges::range Range>
        auto upload(BufferHandle dstHandle, const Range& range, u32 dstOffset = 0) -> u32 {
            return upload(dstHandle, std::span<const u8>(reinterpret_cast<const u8*>(std::ranges::data(range)), std::ranges::size(range) * sizeof(std::ranges::range_value_t<Range>)), dstOffset);
        }

        struct ImageInfo {
            u32 width = 1;
            u32 height = 1;
            u32 depth = 1;
            Format format = Format::RGBA8_UNORM;
            u32 mipLevel = 0;
            u32 layer = 0;
            bool final = true;
        };

        auto upload(ImageHandle dstHandle, std::span<const u8> data, ImageInfo info) -> u32;

        template <std::ranges::range Range>
        auto upload(ImageHandle dstHandle, const Range& range, ImageInfo info) -> u32 {
            return upload(dstHandle, std::span<const u8>(reinterpret_cast<const u8*>(std::ranges::data(range)), std::ranges::size(range) * sizeof(std::ranges::range_value_t<Range>)), info);
        }

        auto flushStagedData() -> UploadBuffer&;

        void wait(u64 timeout = 1000000000);

        auto submitted() const -> std::span<const u64> { return _submitted; }

        auto timeline() const -> const SemaphoreHandle { return _timelineSemaphore; }

        auto clearSubmitted() -> u32;

        auto releasedImages() -> std::vector<ImageBarrier>;

    private:

        Device* _device = nullptr;
        CommandPool _commandPool = {};
        SemaphoreHandle _timelineSemaphore = {};

        BufferHandle _buffer = {};
        u32 _offset = 0;

        struct StagedBufferInfo {
            BufferHandle dst = {};
            u32 dstOffset = 0;
            u32 srcSize = 0;
            u32 srcOffset = 0;
        };
        std::vector<StagedBufferInfo> _pendingStagedBufferCopies = {};
        struct StagedImageInfo {
            ImageHandle dst = {};
            ende::math::Vec<3, u32> dstDimensions = { 0, 0, 0 };
            ende::math::Vec<3, u32> dstOffsets = { 0, 0, 0 };
            u32 dstMipLevel = 0;
            u32 dstLayer = 0;
            u32 dstLayerCount = 1;
            u32 srcSize = 0;
            u32 srcOffset = 0;
            bool finalTransfer = false;
        };
        std::vector<StagedImageInfo> _pendingStagedImageCopies = {};
        std::vector<ImageBarrier> _releasedFromQueue = {};

        std::vector<u64> _submitted = {};

        std::unique_ptr<std::mutex> _mutex = nullptr;

    };

}

#endif //CANTA_UPLOADBUFFER_H
