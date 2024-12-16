#ifndef CANTA_UTIL_H
#define CANTA_UTIL_H

#include <Ende/platform.h>
#include <expected>
#include <vector>
#include <string>
#include <string_view>
#include <span>
#include <Canta/Enums.h>

namespace canta::util {

    struct Macro {
        std::string name;
        std::string value;
    };

    auto compileGLSLToSpirv(std::string_view name, std::string_view glsl, ShaderStage stage, std::span<Macro> macros = {}) -> std::expected<std::vector<u32>, std::string>;

    template<class... Ts>
    constexpr auto maxSize() noexcept -> size_t {
        size_t _maxSize{ 0 };
        ((_maxSize = std::max(_maxSize, sizeof(Ts))), ...);
        return _maxSize;
    };

//#ifndef NDEBUG

#define STRINGIFY(str) #str
#define TO_STRING_ENUM(e) case e: return STRINGIFY(e);

    enum class FunctionMarker {
        None,
        Draw,
        DrawIndexed,
        DrawIndirect,
        DrawIndirectCount,
        DrawIndexedIndirect,
        DrawIndexedIndirectCount,
        MeshTasks,
        MeshTasksIndirect,
        MeshTasksIndirectCount,
        Dispatch,
        DispatchIndirect,
    };

    constexpr const char* markerString(FunctionMarker func) {
        switch (func) {
            TO_STRING_ENUM(FunctionMarker::None)
            TO_STRING_ENUM(FunctionMarker::Draw)
            TO_STRING_ENUM(FunctionMarker::DrawIndexed)
            TO_STRING_ENUM(FunctionMarker::DrawIndirect)
            TO_STRING_ENUM(FunctionMarker::DrawIndirectCount)
            TO_STRING_ENUM(FunctionMarker::DrawIndexedIndirect)
            TO_STRING_ENUM(FunctionMarker::DrawIndexedIndirectCount)
            TO_STRING_ENUM(FunctionMarker::MeshTasks)
            TO_STRING_ENUM(FunctionMarker::MeshTasksIndirect)
            TO_STRING_ENUM(FunctionMarker::MeshTasksIndirectCount)
            TO_STRING_ENUM(FunctionMarker::Dispatch)
            TO_STRING_ENUM(FunctionMarker::DispatchIndirect)
        }
        return STRINGIFY(FunctionMarker::None);
    }

    struct Marker {
        FunctionMarker function = FunctionMarker::Draw;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
    };

    struct Draw {
        FunctionMarker function = FunctionMarker::Draw;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        u32 count = 0;
        u32 instanceCount = 0;
        u32 firstVertex = 0;
        u32 firstInstance = 0;
    };

    struct DrawIndexed {
        FunctionMarker function = FunctionMarker::DrawIndexed;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        u32 count = 0;
        u32 instanceCount = 0;
        u32 firstVertex = 0;
        u32 firstIndex = 0;
        u32 firstInstance = 0;
    };

    struct DrawIndirect {
        FunctionMarker function = FunctionMarker::DrawIndirect;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
        u32 drawCount = 0;
        u32 stride = 0;
    };

    struct DrawIndirectCount {
        FunctionMarker function = FunctionMarker::DrawIndirectCount;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
        i32 countBufferIndex = 0;
        u32 countOffset = 0;
        u32 maxDrawCount = 0;
        u32 stride = 0;
    };

    struct DrawIndexedIndirect {
        FunctionMarker function = FunctionMarker::DrawIndexedIndirect;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
        u32 drawCount = 0;
        u32 stride = 0;
    };

    struct DrawIndexedIndirectCount {
        FunctionMarker function = FunctionMarker::DrawIndexedIndirectCount;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
        i32 countBufferIndex = 0;
        u32 countOffset = 0;
        u32 maxDrawCount = 0;
        u32 stride = 0;
    };

    struct MeshTasks {
        FunctionMarker function = FunctionMarker::MeshTasks;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        u32 x = 0;
        u32 y = 0;
        u32 z = 0;
    };

    struct MeshTasksIndirect {
        FunctionMarker function = FunctionMarker::MeshTasksIndirect;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
        u32 drawCount = 0;
    };

    struct MeshTasksIndirectCount {
        FunctionMarker function = FunctionMarker::MeshTasksIndirectCount;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
        i32 countBufferIndex = 0;
        u32 countOffset = 0;
        u32 maxDrawCount = 0;
        u32 stride = 0;
    };

    struct Dispatch {
        FunctionMarker function = FunctionMarker::Dispatch;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        u32 x = 0;
        u32 y = 0;
        u32 z = 0;
    };

    struct DispatchIndirect {
        FunctionMarker function = FunctionMarker::DispatchIndirect;
        PipelineStage stage = PipelineStage::ALL_COMMANDS;
        i32 bufferIndex = 0;
        u32 offset = 0;
    };

    constexpr const size_t debugMarkerSize = maxSize<Marker, Draw, DrawIndexed, DrawIndirect,
            DrawIndirectCount, DrawIndexedIndirect, DrawIndexedIndirectCount,
            MeshTasks, MeshTasksIndirect, MeshTasksIndirectCount,
            Dispatch, DispatchIndirect>();

    template <typename T>
    constexpr std::array<u8, debugMarkerSize> storeMarker(T marker) {
        std::array<u8, debugMarkerSize> data = {};
        for (size_t i = 0; i < sizeof(T); i++) {
            data[i] = reinterpret_cast<u8*>(&marker)[i];
        }
        return data;
    }

    static auto getMarkerType(const std::array<u8, debugMarkerSize>& data) -> FunctionMarker {
        return reinterpret_cast<const Marker*>(&data)->function;
    }

    static auto getMarkerStage(const std::array<u8, debugMarkerSize>& data) -> PipelineStage {
        return reinterpret_cast<const Marker*>(&data)->stage;
    }
//#else
//
//    constexpr const size_t debugMarkerSize = 0;
//
//#endif

}

#endif //CANTA_UTIL_H
