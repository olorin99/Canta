#ifndef CANTA_BUFFER_H
#define CANTA_BUFFER_H

#include <Ende/platform.h>
#include <volk.h>
#include <vk_mem_alloc.h>
#include <Canta/Enums.h>
#include <string>
#include <span>

namespace canta {

    class Device;

    class Buffer {
    public:

        struct CreateInfo {
            u32 size = 0;
            BufferUsage usage = BufferUsage::TRANSFER_DST;
            MemoryType type = MemoryType::DEVICE;
            u32 requiredFlags = 0;
            u32 preferredFlags = 0;
            bool persistentlyMapped = false;
            std::string_view name = {};
        };

        Buffer() = default;

        ~Buffer();

        Buffer(Buffer&& rhs) noexcept;
        auto operator=(Buffer&& rhs) noexcept -> Buffer&;

        auto buffer() const -> VkBuffer { return _buffer; }
        auto address() const -> u64 { return _deviceAddress; }
        auto type() const -> MemoryType { return _type; }
        auto usage() const -> BufferUsage { return _usage; }
        auto size() const -> u32 { return _size; }
        auto persitentlyMapped() const -> bool { return _mapped._address; }
        auto name() const -> std::string_view { return _name; }

        class Mapped {
        public:

            Mapped() = default;
            ~Mapped();

            Mapped(Mapped&& rhs) noexcept;
            auto operator=(Mapped&& rhs) noexcept -> Mapped&;

            auto address() -> void* { return _address; }

        private:
            friend Buffer;
            friend Device;

            Device* _device = nullptr;
            void* _address = nullptr;
            Buffer* _buffer = nullptr;
        };

        auto map(u32 offset = 0, u32 size = 0) -> Mapped;

        void data(std::span<const u8> data, u32 offset = 0);

        template <std::ranges::range Range>
        void data(const Range& range, u32 offset = 0) {
            data(std::span<const u8>(reinterpret_cast<const u8*>(std::ranges::data(range)), std::ranges::size(range) * sizeof(std::ranges::range_value_t<Range>)), offset);
        }

    private:
        friend Device;

        Device* _device = nullptr;
        VkBuffer _buffer = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        VkDeviceAddress _deviceAddress = 0;
        u32 _size = 0;
        BufferUsage _usage = BufferUsage::TRANSFER_DST;
        MemoryType _type = MemoryType::DEVICE;
        Mapped _mapped = {};
        u32 _requiredFlags = 0;
        u32 _preferredFlags = 0;
        std::string _name = {};

    };

}

#endif //CANTA_BUFFER_H
