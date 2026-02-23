#ifndef CANTA_KERNELHELPER_H
#define CANTA_KERNELHELPER_H

#include <Canta/RenderGraph.h>

namespace canta::util {

    struct kernel_helper {

        kernel_helper(const std::string_view name, const std::span<const u32> source)
            : _name(name), _source(source)
        {}

        const std::string _name;
        const std::span<const u32> _source;

        PipelineHandle _pipeline = {};

        u32 _x = 1;
        u32 _y = 1;
        u32 _z = 1;

        auto pipeline(PipelineManager& manager) -> kernel_helper& {
            auto source = std::vector<u32>{};
            source.assign(_source.begin(), _source.end());

            _pipeline = manager.getPipeline({
                .compute = {
                    .module = manager.getShader({
                        .spirv = source,
                        .stage = canta::ShaderStage::COMPUTE,
                        .name = _name
                    }).value()
                }
            }).value();
            return *this;
        }

        auto pipeline(Device* device) -> kernel_helper& {
            auto source = std::vector<u32>{};
            source.assign(_source.begin(), _source.end());

            _pipeline = device->createPipeline({
                .compute = {
                    .module = device->createShaderModule({
                        .spirv = source,
                        .stage = canta::ShaderStage::COMPUTE,
                        .name = _name
                    })
                }
            });
            return *this;
        }

        auto operator()(PipelineManager& manager) -> kernel_helper& {
            return pipeline(manager);
        }

        auto operator()(const u32 x = 1, const u32 y = 1, const u32 z = 1) -> kernel_helper& {
            _x = x;
            _y = y;
            _z = z;
            return *this;
        }

        template <typename... Args>
        auto operator()(RenderGraph& graph, Args&&... args) -> canta::RenderPass& {
            if (!_pipeline) {
                return pipeline(graph.device())(graph, std::forward<Args>(args)...);
            }

            return graph.addPass({.name = _name})
                .setPipeline(_pipeline)
                .pushConstants(std::forward<Args>(args)...)
                .dispatchThreads(_x, _y, _z);
        }

    };

}

#endif //CANTA_KERNELHELPER_H