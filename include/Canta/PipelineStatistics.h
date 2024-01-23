#ifndef CANTA_PIPELINESTATISTICS_H
#define CANTA_PIPELINESTATISTICS_H

#include <Ende/platform.h>
#include <Canta/Enums.h>
#include <expected>

namespace canta {

    class Device;
    class CommandBuffer;

    class PipelineStatistics {
    public:

        PipelineStatistics() = default;
        ~PipelineStatistics();

        PipelineStatistics(PipelineStatistics&& rhs) noexcept;
        auto operator=(PipelineStatistics&& rhs) noexcept -> PipelineStatistics&;

        void begin(CommandBuffer& commandBuffer);
        void end(CommandBuffer& commandBuffer);

        struct Stats {
            u64 inputAssemblyVertices = 0;
            u64 inputAssemblyPrimitives = 0;
            u64 vertexShaderInvocations = 0;
            u64 geometryShaderInvocations = 0;
            u64 geometryShaderPrimitives = 0;
            u64 clippingInvocations = 0;
            u64 clippingPrimitives = 0;
            u64 fragmentShaderInvocations = 0;
            u64 tessellationControlShaderPatches = 0;
            u64 tessellationEvaluationShaderInvocations = 0;
            u64 computeShaderInvocations = 0;
        };
        auto result() -> std::expected<Stats, Error>;

    private:
        friend Device;

        Device* _device = nullptr;
        u32 _queryPoolIndex = 0;
        u32 _index = 0;
        u32 _queryCount = 0;
        Stats _value = {};

    };

}

#endif //CANTA_PIPELINESTATISTICS_H
