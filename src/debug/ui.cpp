#include <Canta/debug/ui.h>
#include <imgui/imgui.h>

auto canta::drawPipelineStats(const PipelineStatistics::Stats &stats, const std::string_view name) -> bool {
    const auto value = name.empty() ? true : ImGui::Begin(name.data());
    if (value) {
        ImGui::Text("Input Assembly Vertices: %lu", stats.inputAssemblyVertices);
        ImGui::Text("Input Assembly Primitives: %lu", stats.inputAssemblyPrimitives);
        ImGui::Text("Vertex Shader Invocations: %lu", stats.vertexShaderInvocations);
        ImGui::Text("Geometry Shader Invocations: %lu", stats.geometryShaderInvocations);
        ImGui::Text("Geometry Shader Primitives: %lu", stats.geometryShaderPrimitives);
        ImGui::Text("Clipping Invocations: %lu", stats.clippingInvocations);
        ImGui::Text("Clipping Primitives: %lu", stats.clippingPrimitives);
        ImGui::Text("Fragment Shader Invocations: %lu", stats.fragmentShaderInvocations);
        ImGui::Text("Tessellation Control Shader Patches: %lu", stats.tessellationControlShaderPatches);
        ImGui::Text("Tessellation Evaluation Shader Invocations: %lu", stats.tessellationEvaluationShaderInvocations);
        ImGui::Text("Compute Shader Invocations: %lu", stats.computeShaderInvocations);
    }
    if (!name.empty())
        ImGui::End();
    return value;
}

auto canta::drawResourceStats(const Device::ResourceStats &stats, std::string_view name) -> bool {
    const auto value = name.empty() ? true : ImGui::Begin(name.data());
    if (value) {
        // ImGui::Text("Resource timeline value %lu", device->resourceTimeline()->value());
        ImGui::Text("Shader Count: %d", stats.shaderCount);
        ImGui::Text("Shader Allocated: %d", stats.shaderAllocated);
        ImGui::Text("Pipeline Count: %d", stats.pipelineCount);
        ImGui::Text("Pipeline Allocated: %d", stats.pipelineAllocated);
        ImGui::Text("Image Count: %d", stats.imageCount);
        ImGui::Text("Image Allocated: %d", stats.imageAllocated);
        ImGui::Text("Buffer Count: %d", stats.bufferCount);
        ImGui::Text("Buffer Allocated: %d", stats.bufferAllocated);
        ImGui::Text("Sampler Count: %d", stats.samplerCount);
        ImGui::Text("Sampler Allocated: %d", stats.shaderAllocated);
        ImGui::Text("Timestamp Query Pools: %d", stats.timestampQueryPools);
        ImGui::Text("PipelineStats Pools: %d", stats.pipelineStatsPools);
    }
    if (!name.empty())
        ImGui::End();
    return value;
}

auto canta::drawMemoryUsage(const Device::MemoryUsage &usage, std::string_view name) -> bool {
    const auto value = name.empty() ? true : ImGui::Begin(name.data());
    if (value) {
        ImGui::Text("VRAM Budget: %lu mb", usage.budget / 1000000);
        ImGui::Text("VRAM Usage: %lu mb", usage.usage / 1000000);
        ImGui::Text("VRAM Usage: %f%%", static_cast<f64>(usage.usage) / static_cast<f64>(usage.budget));
    }
    if (!name.empty())
        ImGui::End();
    return value;
}
