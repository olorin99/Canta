#include "Canta/Buffer.h"
#include <Canta/Device.h>
#include <cstring>

canta::Buffer::~Buffer() {
    if (!_device)
        return;
    _mapped = {};
    vmaDestroyBuffer(_device->allocator(), _buffer, _allocation);
}

canta::Buffer::Buffer(canta::Buffer &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_buffer, rhs._buffer);
    std::swap(_allocation, rhs._allocation);
    std::swap(_deviceAddress, rhs._deviceAddress);
    std::swap(_size, rhs._size);
    std::swap(_usage, rhs._usage);
    std::swap(_type, rhs._type);
    std::swap(_mapped, rhs._mapped);
    _mapped._buffer = this;
    rhs._mapped._buffer = &rhs;
    std::swap(_requiredFlags, rhs._requiredFlags);
    std::swap(_preferredFlags, rhs._preferredFlags);
    std::swap(_name, rhs._name);
}

auto canta::Buffer::operator=(canta::Buffer &&rhs) noexcept -> Buffer & {
    std::swap(_device, rhs._device);
    std::swap(_buffer, rhs._buffer);
    std::swap(_allocation, rhs._allocation);
    std::swap(_deviceAddress, rhs._deviceAddress);
    std::swap(_size, rhs._size);
    std::swap(_usage, rhs._usage);
    std::swap(_type, rhs._type);
    std::swap(_mapped, rhs._mapped);
    _mapped._buffer = this;
    rhs._mapped._buffer = &rhs;
    std::swap(_requiredFlags, rhs._requiredFlags);
    std::swap(_preferredFlags, rhs._preferredFlags);
    std::swap(_name, rhs._name);
    return *this;
}

canta::Buffer::Mapped::~Mapped() {
    if (!_device || !_buffer)
        return;
    vmaUnmapMemory(_device->allocator(), _buffer->_allocation);
}

canta::Buffer::Mapped::Mapped(canta::Buffer::Mapped &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_address, rhs._address);
    std::swap(_buffer, rhs._buffer);
}

auto canta::Buffer::Mapped::operator=(canta::Buffer::Mapped &&rhs) noexcept -> Mapped & {
    std::swap(_device, rhs._device);
    std::swap(_address, rhs._address);
    std::swap(_buffer, rhs._buffer);
    return *this;
}

auto canta::Buffer::map(u32 offset, u32 size) -> Mapped {
    if (size == 0)
        size = _size - offset;

    assert(_size >= size + offset);
    void* address = nullptr;
    VK_TRY(vmaMapMemory(_device->allocator(), _allocation, &address));
    assert(address);
    Mapped mapped = {};
    mapped._device = _device;
    mapped._address = static_cast<char*>(address) + offset;
    mapped._buffer = this;
    return mapped;
}

auto canta::Buffer::data(std::span<const u8> data, u32 offset) -> u32 {
    if (_mapped._address)
        std::memcpy(static_cast<char*>(_mapped.address()) + offset, data.data(), data.size());
    else {
        auto mapped = map(offset, data.size());
        std::memcpy(mapped.address(), data.data(), data.size());
    }
    return data.size();
}