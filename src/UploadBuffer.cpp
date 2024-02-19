#include <cstring>
#include "Canta/UploadBuffer.h"

auto canta::UploadBuffer::create(canta::UploadBuffer::CreateInfo info) -> UploadBuffer {
    UploadBuffer buffer = {};

    buffer._device = info.device;
    buffer._commandPool = info.device->createCommandPool({
        .queueType = QueueType::TRANSFER,
        .name = "upload_buffer_command_pool"
    }).value();
    buffer._timelineSemaphore = info.device->createSemaphore({
        .initialValue = 0,
        .name = "upload_buffer_semaphore"
    }).value();
    buffer._buffer = info.device->createBuffer({
        .size = info.size,
        .usage = BufferUsage::TRANSFER_SRC | BufferUsage::TRANSFER_DST,
        .type = MemoryType::STAGING,
        .persistentlyMapped = true,
        .name = "upload_buffer"
    });
    buffer._mutex = std::make_unique<std::mutex>();

    return buffer;
}

canta::UploadBuffer::~UploadBuffer() {

}

canta::UploadBuffer::UploadBuffer(canta::UploadBuffer &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_commandPool, rhs._commandPool);
    std::swap(_timelineSemaphore, rhs._timelineSemaphore);
    std::swap(_buffer, rhs._buffer);
    std::swap(_offset, rhs._offset);
    std::swap(_pendingStagedBufferCopies, rhs._pendingStagedBufferCopies);
    std::swap(_pendingStagedImageCopies, rhs._pendingStagedImageCopies);
    std::swap(_submitted, rhs._submitted);
    std::swap(_mutex, rhs._mutex);
}

auto canta::UploadBuffer::operator=(canta::UploadBuffer &&rhs) noexcept -> UploadBuffer & {
    std::swap(_device, rhs._device);
    std::swap(_commandPool, rhs._commandPool);
    std::swap(_timelineSemaphore, rhs._timelineSemaphore);
    std::swap(_buffer, rhs._buffer);
    std::swap(_offset, rhs._offset);
    std::swap(_pendingStagedBufferCopies, rhs._pendingStagedBufferCopies);
    std::swap(_pendingStagedImageCopies, rhs._pendingStagedImageCopies);
    std::swap(_submitted, rhs._submitted);
    std::swap(_mutex, rhs._mutex);
    return *this;
}

auto canta::UploadBuffer::upload(canta::BufferHandle dstHandle, std::span<const u8> data, u32 dstOffset) -> u32 {
    u32 uploadOffset = 0;
    i32 uploadSizeRemaining = data.size() - uploadOffset;

    while (uploadSizeRemaining > 0) {
        std::unique_lock lock(*_mutex);
        u32 availableSize = _buffer->size() - _offset;
        u32 allocationSize = std::min(availableSize, static_cast<u32>(uploadSizeRemaining));

        if (allocationSize > 0) {
            std::memcpy(static_cast<u8*>(_buffer->mapped().address()) + _offset, data.data() + uploadOffset, allocationSize);

            _pendingStagedBufferCopies.push_back({
                .dst = dstHandle,
                .dstOffset = dstOffset,
                .srcSize = allocationSize,
                .srcOffset = _offset
            });

            _offset += allocationSize;
            uploadOffset += allocationSize;
            dstOffset += allocationSize;
            uploadSizeRemaining = data.size() - uploadOffset;
        } else {
            lock.unlock();
            flushStagedData();
            wait();
        }
    }
    return data.size();
}

auto canta::UploadBuffer::upload(canta::ImageHandle dstHandle, std::span<const u8> data, canta::UploadBuffer::ImageInfo info) -> u32 {
    u32 uploadOffset = 0;
    u32 uploadSizeRemaining = data.size() - uploadOffset;
    ende::math::Vec<3, u32> dstOffset = { 0, 0, 0 };

    //TODO: support loading 3d images

    while (uploadSizeRemaining > 0) {
        std::unique_lock lock(*_mutex);
        u32 availableSize = _buffer->size() - _offset;
        u32 allocationSize = std::min(availableSize, uploadSizeRemaining);

        if (allocationSize > 0 && allocationSize >= formatSize(info.format)) {
            u32 allocIndex = allocationSize / formatSize(info.format);
            u32 regionZ = 0;
            if (info.depth > 1) {
                regionZ = allocIndex / (info.width * info.height);
                allocIndex -= (regionZ * (info.width * info.height));
            }
            u32 regionY = allocIndex / info.width;

            // find square copy region
            // get space remaining on current line. if this doesn't cover the whole line then only fill line
            u32 rWidth = std::min(info.width - dstOffset.x(), allocIndex);
            u32 rHeight = regionY;
            if (rWidth < info.width)
                rHeight = 1;


            u32 allocSize = rWidth * rHeight * formatSize(info.format);
            allocationSize = allocSize;

            u32 remainder = _offset % formatSize(info.format);

            u32 offset = _offset;
            if (remainder != 0)
                offset += (formatSize(info.format) - remainder);

            std::memcpy(static_cast<u8*>(_buffer->mapped().address()) + offset, data.data() + uploadOffset, allocationSize);

            _pendingStagedImageCopies.push_back({
                .dst = dstHandle,
                .dstDimensions = { rWidth, rHeight, info.depth },
                .dstOffsets = dstOffset,
                .dstMipLevel = info.mipLevel,
                .dstLayer = info.layer,
                .dstLayerCount = 1,
                .srcSize = allocationSize,
                .srcOffset = offset,
                .finalTransfer = data.size() - (uploadOffset + allocationSize) == 0
            });

            _offset = offset + allocationSize;
            uploadOffset += allocationSize;
            uploadSizeRemaining = data.size() - uploadOffset;

            u32 index = (data.size() - uploadSizeRemaining) / formatSize(info.format);
            u32 z = 0;
            if (info.depth > 1) {
                z = index / (info.width * info.height);
                index -= (z * (info.width * info.height));
            }
            u32 y = 0;
            if (info.height > 1)
                y = index / info.width;
            u32 x = index % info.width;

            dstOffset = { x, y, z };

        } else {
            lock.unlock();
            flushStagedData();
            wait();
        }
    }
    return data.size();
}

auto canta::UploadBuffer::flushStagedData() -> UploadBuffer& {
    std::unique_lock lock(*_mutex);
    if (!_pendingStagedBufferCopies.empty() || !_pendingStagedImageCopies.empty()) {
        auto& commandBuffer = _commandPool.getBuffer();
        commandBuffer.begin();
        if (!_pendingStagedBufferCopies.empty()) {
            for (auto& staged : _pendingStagedBufferCopies) {
                commandBuffer.copyBuffer({
                    .src = _buffer,
                    .dst = staged.dst,
                    .srcOffset = staged.srcOffset,
                    .dstOffset = staged.dstOffset,
                    .size = staged.srcSize
                });
            }
        }
        if (!_pendingStagedImageCopies.empty()) {
            for (auto& staged : _pendingStagedImageCopies) {
                //TODO: manager barriers
                commandBuffer.barrier({
                    .image = staged.dst,
                    .srcStage = PipelineStage::TOP,
                    .dstStage = PipelineStage::TRANSFER,
                    .srcAccess = Access::NONE,
                    .dstAccess = Access::TRANSFER_WRITE,
                    .srcLayout = ImageLayout::UNDEFINED,
                    .dstLayout = ImageLayout::TRANSFER_DST
                });
                commandBuffer.copyBufferToImage({
                    .src = _buffer,
                    .dst = staged.dst,
                    .dstLayout = ImageLayout::TRANSFER_DST,
                    .dstDimensions = staged.dstDimensions,
                    .dstOffsets = staged.dstOffsets,
                    .dstMipLevel = staged.dstMipLevel,
                    .dstLayer = staged.dstLayer,
                    .dstLayerCount = staged.dstLayerCount,
                    .size = staged.srcSize,
                    .srcOffset = staged.srcOffset
                });
                if (staged.finalTransfer) {
                    auto barrier = ImageBarrier {
                            .image = staged.dst,
                            .srcStage = PipelineStage::TRANSFER,
                            .dstStage = PipelineStage::BOTTOM,
                            .srcAccess = Access::TRANSFER_WRITE,
                            .dstAccess = Access::MEMORY_READ,
                            .srcLayout = ImageLayout::TRANSFER_DST,
                            .dstLayout = ImageLayout::SHADER_READ_ONLY,
                            .srcQueue = _device->queue(QueueType::TRANSFER).familyIndex(),
                            .dstQueue = _device->queue(QueueType::GRAPHICS).familyIndex()
                    };
                    commandBuffer.barrier(barrier);
                    _releasedFromQueue.push_back(barrier);
                }
            }
        }
        commandBuffer.end();
        auto waits = std::to_array({
            _timelineSemaphore.getPair()
        });
        _timelineSemaphore.increment();
        auto signals = std::to_array({
            _timelineSemaphore.getPair()
        });
        commandBuffer.submit(waits, signals);
        _submitted.push_back(_timelineSemaphore.value());
    }

    _pendingStagedBufferCopies.clear();
    _pendingStagedImageCopies.clear();
    _offset = 0;
    return *this;
}

void canta::UploadBuffer::wait(u64 timeout) {
    if (_submitted.empty())
        return;
    auto maxSignal = std::max_element(_submitted.begin(), _submitted.end());
    _timelineSemaphore.wait(*maxSignal);
}

auto canta::UploadBuffer::clearSubmitted() -> u32 {
    auto gpuTimelineValue = _timelineSemaphore.gpuValue();
    u32 clearedCount = 0;
    std::unique_lock lock(*_mutex);
    for (auto it = _submitted.begin(); it != _submitted.end(); it++) {
        if (*it < gpuTimelineValue) {
            _submitted.erase(it--);
            clearedCount++;
        }
    }
    return clearedCount;
}

auto canta::UploadBuffer::releasedImages() -> std::vector<ImageBarrier> {
    std::unique_lock lock(*_mutex);
    auto tmp = _releasedFromQueue;
    _releasedFromQueue.clear();
    return tmp;
}