#include <Canta/RenderGraphV2.h>

constexpr auto defaultPassStage(const canta::V2::RenderPass::Type type) -> canta::PipelineStage {
    switch (type) {
        case canta::V2::RenderPass::Type::GRAPHICS:
            return canta::PipelineStage::ALL_GRAPHICS;
        case canta::V2::RenderPass::Type::COMPUTE:
            return canta::PipelineStage::COMPUTE_SHADER;
        case canta::V2::RenderPass::Type::TRANSFER:
            return canta::PipelineStage::TRANSFER;
        case canta::V2::RenderPass::Type::HOST:
            return canta::PipelineStage::HOST;
    }
    return canta::PipelineStage::ALL_COMMANDS;
}

constexpr auto checkPassStageMatch(const canta::V2::RenderPass::Type type, const canta::PipelineStage stage) -> bool {
    switch (type) {
        case canta::V2::RenderPass::Type::GRAPHICS:
            switch (stage) {
            case canta::PipelineStage::TOP:
                    return true;
            }
            break;
        case canta::V2::RenderPass::Type::COMPUTE:
            switch (stage) {
            case canta::PipelineStage::COMPUTE_SHADER:
                    return true;
            }
            break;
        case canta::V2::RenderPass::Type::TRANSFER:
            switch (stage) {
            case canta::PipelineStage::COMPUTE_SHADER:
                    return true;
            }
            break;
        case canta::V2::RenderPass::Type::HOST:
            if (stage != canta::PipelineStage::HOST) return false;
            break;
    }
    return false;
}




auto canta::V2::RenderPass::clone() -> RenderPass {
    return *this;
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const BufferIndex &index) {
    _deferredPushConstants.push_back(DeferredPushConstant{
        .type = 0,
        .value = index,
        .offset = i
    });
    i += sizeof(u64);
    dst.size += sizeof(u64);
    assert(i <= 128);
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const ImageIndex &index) {
    _deferredPushConstants.emplace_back(DeferredPushConstant{
        .type = 1,
        .value = index,
        .offset = i,
    });
    i += sizeof(u32);
    dst.size += sizeof(u32);
    assert(i <= 128);
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const BufferHandle &handle) {
    const auto addr = handle->address();
    unpack(dst, i, addr);
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const ImageHandle &handle) {
    const auto id = handle->defaultView().index();
    unpack(dst, i, id);
}

void unpack(canta::V2::RenderPass::PushData &dst, i32 &i, const canta::BufferHandle &handle) {
    const auto addr = handle->address();
    auto* data = reinterpret_cast<const u8*>(&addr);
    for (auto j = 0; j < sizeof(addr); j++) {
        dst.data[i + j] = data[j];
    }
    i += sizeof(addr);
    dst.size += sizeof(addr);
}

void unpack(canta::V2::RenderPass::PushData &dst, i32 &i, const canta::ImageHandle &handle) {
    const auto id = handle->defaultView().index();
    auto* data = reinterpret_cast<const u8*>(&id);
    for (auto j = 0; j < sizeof(id); j++) {
        dst.data[i + j] = data[j];
    }
    i += sizeof(id);
    dst.size += sizeof(id);
}

auto canta::V2::RenderPass::setCallback(const std::function<void(CommandBuffer&, RenderGraph&, const PushData&)> &callback) -> RenderPass & {
    _callback = callback;
    return *this;
}

auto canta::V2::RenderPass::resolvePushConstants(canta::V2::RenderGraph& graph, PushData data, const std::span<DeferredPushConstant> deferredConstants) -> PushData {
    for (auto& deferredConstant : deferredConstants) {
        if (deferredConstant.type == 0)
            ::unpack(data, deferredConstant.offset, graph.getBuffer(std::get<BufferIndex>(deferredConstant.value)));
        else
            ::unpack(data, deferredConstant.offset, graph.getImage(std::get<ImageIndex>(deferredConstant.value)));
    }
    return data;
}

canta::V2::PassBuilder::PassBuilder(RenderGraph *graph, const u32 index) : _graph(graph), _vertexIndex(index) {}

auto canta::V2::PassBuilder::pass() -> RenderPass& {
    return _graph->getVertices()[_vertexIndex];
}

auto canta::V2::PassBuilder::read(const BufferIndex index, const Access access, const PipelineStage stage) -> bool {
    pass().inputs.emplace_back(index);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = index.id,
        .index = index.index,
        .access = access,
        .stage = stage
    });
    return true;
}

auto canta::V2::PassBuilder::read(const ImageIndex index, const Access access, const PipelineStage stage, const ImageLayout layout) -> bool {
    pass().inputs.emplace_back(index);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = index.id,
        .index = index.index,
        .access = access,
        .stage = stage,
        .layout = layout,
    });
    return true;
}

auto canta::V2::PassBuilder::write(const BufferIndex index, const Access access, const PipelineStage stage) -> bool {
    const auto alias = _graph->alias(index);
    pass().outputs.emplace_back(alias);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = alias.id,
        .index = alias.index,
        .access = access,
        .stage = stage
    });
    return true;
}

auto canta::V2::PassBuilder::write(const ImageIndex index, const Access access, const PipelineStage stage, const ImageLayout layout) -> bool {
    const auto alias = _graph->alias(index);
    pass().outputs.emplace_back(alias);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = alias.id,
        .index = alias.index,
        .access = access,
        .stage = stage,
        .layout = layout,
    });
    return true;
}


auto canta::V2::PassBuilder::addColourRead(const ImageIndex index) -> PassBuilder& {
    read(index, Access::COLOUR_READ, PipelineStage::COLOUR_OUTPUT, ImageLayout::COLOUR_ATTACHMENT);
    return *this;
}

auto canta::V2::PassBuilder::addColourWrite(const ImageIndex index, const ClearValue& clearColour) -> PassBuilder& {
    write(index, Access::COLOUR_READ | Access::COLOUR_WRITE, PipelineStage::COLOUR_OUTPUT, ImageLayout::COLOUR_ATTACHMENT);
    return *this;
}

auto canta::V2::PassBuilder::addDepthRead(const ImageIndex index) -> PassBuilder& {
    read(index, Access::DEPTH_STENCIL_READ, PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER, ImageLayout::DEPTH_STENCIL_ATTACHMENT);
    return *this;
}

auto canta::V2::PassBuilder::addDepthWrite(const ImageIndex index, const ClearValue& clearColour) -> PassBuilder& {
    write(index, Access::DEPTH_STENCIL_READ | Access::DEPTH_STENCIL_WRITE, PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER, ImageLayout::DEPTH_STENCIL_ATTACHMENT);
    return *this;
}

auto canta::V2::PassBuilder::addStorageImageRead(const ImageIndex index, const PipelineStage stage) -> PassBuilder & {
    read(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ, stage, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::PassBuilder::addStorageImageWrite(const ImageIndex index, const PipelineStage stage) -> PassBuilder & {
    write(index, stage == PipelineStage::HOST ? Access::HOST_READ | Access::HOST_WRITE : Access::SHADER_READ | Access::SHADER_WRITE, stage, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::PassBuilder::addStorageBufferRead(const BufferIndex index, const PipelineStage stage) -> PassBuilder & {
    read(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ, stage);
    return *this;
}

auto canta::V2::PassBuilder::addStorageBufferWrite(const BufferIndex index, const PipelineStage stage) -> PassBuilder & {
    write(index, stage == PipelineStage::HOST ? Access::HOST_READ | Access::HOST_WRITE : Access::SHADER_READ | Access::SHADER_WRITE, stage);
    return *this;
}

auto canta::V2::PassBuilder::addSampledRead(const ImageIndex index, const PipelineStage stage) -> PassBuilder& {
    read(index, Access::SHADER_READ, stage, ImageLayout::SHADER_READ_ONLY);
    return *this;
}

auto canta::V2::PassBuilder::addBlitRead(const ImageIndex index) -> PassBuilder& {
    read(index, Access::TRANSFER_READ, PipelineStage::TRANSFER, ImageLayout::TRANSFER_SRC);
    return *this;
}

auto canta::V2::PassBuilder::addBlitWrite(const ImageIndex index) -> PassBuilder& {
    write(index, Access::TRANSFER_READ | Access::TRANSFER_WRITE, PipelineStage::TRANSFER, ImageLayout::TRANSFER_DST);
    return *this;
}

auto canta::V2::PassBuilder::addTransferRead(const ImageIndex index) -> PassBuilder& {
    read(index, Access::TRANSFER_READ, PipelineStage::TRANSFER, ImageLayout::TRANSFER_SRC);
    return *this;
}

auto canta::V2::PassBuilder::addTransferWrite(const ImageIndex index) -> PassBuilder& {
    write(index, Access::TRANSFER_READ | Access::TRANSFER_WRITE, PipelineStage::TRANSFER, ImageLayout::TRANSFER_DST);
    return *this;
}

auto canta::V2::PassBuilder::addTransferRead(const BufferIndex index) -> PassBuilder& {
    read(index, Access::TRANSFER_READ, PipelineStage::TRANSFER);
    return *this;
}

auto canta::V2::PassBuilder::addTransferWrite(const BufferIndex index) -> PassBuilder& {
    write(index, Access::TRANSFER_READ | Access::TRANSFER_WRITE, PipelineStage::TRANSFER);
    return *this;
}

auto canta::V2::PassBuilder::addIndirectRead(const BufferIndex index) -> PassBuilder& {
    read(index, Access::INDIRECT, PipelineStage::DRAW_INDIRECT);
    return *this;
}

auto canta::V2::PassBuilder::addDummyRead(const ImageIndex index) -> PassBuilder& {
    read(index, Access::NONE, PipelineStage::NONE, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::PassBuilder::addDummyWrite(const ImageIndex index) -> PassBuilder& {
    write(index, Access::NONE, PipelineStage::NONE, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::PassBuilder::addDummyRead(const BufferIndex index) -> PassBuilder& {
    read(index, Access::NONE, PipelineStage::NONE);
    return *this;
}

auto canta::V2::PassBuilder::addDummyWrite(const BufferIndex index) -> PassBuilder& {
    write(index, Access::NONE, PipelineStage::NONE);
    return *this;
}

canta::V2::ComputePass::ComputePass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::ComputePass::addStorageImageRead(const ImageIndex index) -> ComputePass & {
    PassBuilder::addStorageImageRead(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addStorageImageWrite(const ImageIndex index) -> ComputePass & {
    PassBuilder::addStorageImageWrite(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addStorageBufferRead(const BufferIndex index) -> ComputePass & {
    PassBuilder::addStorageBufferRead(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addStorageBufferWrite(const BufferIndex index) -> ComputePass & {
    PassBuilder::addStorageBufferWrite(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addSampledRead(const ImageIndex index) -> ComputePass& {
    PassBuilder::addSampledRead(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::dispatchThreads(u32 x, u32 y, u32 z) -> ComputePass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchThreads(x, y, z);
    });
    return *this;
}

auto canta::V2::ComputePass::dispatchWorkgroups(u32 x, u32 y, u32 z) -> ComputePass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchWorkgroups(x, y, z);
    });
    return *this;
}

auto canta::V2::ComputePass::dispatchIndirect(BufferHandle commandBuffer, u32 offset) -> ComputePass & {
    pass().setCallback([commandBuffer, offset] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchIndirect(commandBuffer, offset);
    });
    return *this;
}


canta::V2::GraphicsPass::GraphicsPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::GraphicsPass::addColourRead(const ImageIndex index) -> GraphicsPass& {
    PassBuilder::addColourRead(index);
    return *this;
}

auto canta::V2::GraphicsPass::addColourWrite(const ImageIndex index, const ClearValue& clearColour) -> GraphicsPass& {
    PassBuilder::addColourWrite(index, clearColour);
    return *this;
}

auto canta::V2::GraphicsPass::addDepthRead(const ImageIndex index) -> GraphicsPass& {
    PassBuilder::addDepthRead(index);
    return *this;
}

auto canta::V2::GraphicsPass::addDepthWrite(const ImageIndex index, const ClearValue& clearColour) -> GraphicsPass& {
    PassBuilder::addDepthWrite(index, clearColour);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageImageRead(const ImageIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageImageRead(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageImageWrite(const ImageIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageImageWrite(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageBufferRead(const BufferIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageBufferRead(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageBufferWrite(const BufferIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageBufferWrite(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addSampledRead(const ImageIndex index, const PipelineStage stage) -> GraphicsPass& {
    PassBuilder::addSampledRead(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::draw(u32 count, u32 instanceCount, u32 firstVertex, u32 firstIndex, u32 firstInstance, bool indexed) -> GraphicsPass & {
    pass().setCallback([count, instanceCount, firstVertex, firstIndex, firstInstance, indexed] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::VERTEX | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.draw(count, instanceCount, firstVertex, firstIndex, firstInstance, indexed);
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawIndirect(BufferHandle commands, u32 offset, u32 drawCount, bool indexed, u32 stride) -> GraphicsPass & {
    pass().setCallback([commands, offset, drawCount, indexed, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::VERTEX | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawIndirect(commands, offset, drawCount, indexed, stride);
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, bool indexed, u32 stride) -> GraphicsPass & {
    pass().setCallback([commands, offset, countBuffer, countOffset, indexed, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::VERTEX | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawIndirectCount(commands, offset, countBuffer, countOffset, indexed, stride);
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksThreads(u32 x, u32 y, u32 z) -> GraphicsPass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksThreads(x, y, z);
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksWorkgroups(u32 x, u32 y, u32 z) -> GraphicsPass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksWorkgroups(x, y, z);
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksIndirect(BufferHandle commands, u32 offset, u32 drawCount, u32 stride) -> GraphicsPass & {
    pass().setCallback([commands, offset, drawCount, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksIndirect(commands, offset, drawCount, stride);
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, u32 stride) -> GraphicsPass & {
    pass().setCallback([commands, offset, countBuffer, countOffset, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksIndirectCount(commands, offset, countBuffer, countOffset, stride);
    });
    return *this;
}

canta::V2::TransferPass::TransferPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::TransferPass::addTransferRead(const BufferIndex index) -> TransferPass& {
    PassBuilder::addTransferRead(index);
    return *this;
}

auto canta::V2::TransferPass::addTransferWrite(const BufferIndex index) -> TransferPass& {
    PassBuilder::addTransferWrite(index);
    return *this;
}

auto canta::V2::TransferPass::addTransferRead(const ImageIndex index) -> TransferPass& {
    PassBuilder::addTransferRead(index);
    return *this;
}

auto canta::V2::TransferPass::addTransferWrite(const ImageIndex index) -> TransferPass& {
    PassBuilder::addTransferWrite(index);
    return *this;
}

auto canta::V2::TransferPass::blit(const ImageIndex src, const ImageIndex dst, const Filter filter) -> TransferPass& {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, filter] (auto& cmd, auto& graph, const auto& push) {
        cmd.blit({
            .src = graph.getImage(src),
            .dst = graph.getImage(dst),
            .srcLayout = ImageLayout::TRANSFER_SRC,
            .dstLayout = ImageLayout::TRANSFER_DST,
            .filter = filter,
        });
    });
    return *this;
}

auto canta::V2::TransferPass::copy(BufferIndex src, BufferIndex dst, u32 srcOffset, u32 dstOffset, u32 size) -> TransferPass& {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, srcOffset, dstOffset, size] (auto& cmd, auto& graph, const auto& push) {
        cmd.copyBuffer({
            .src = graph.getBuffer(src),
            .dst = graph.getBuffer(dst),
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = size,
        });
    });
    return *this;
}

auto canta::V2::TransferPass::copy(BufferIndex src, ImageIndex dst, ImageCopy info) -> TransferPass& {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, info] (auto& cmd, auto& graph, const auto& push) {
        cmd.copyBufferToImage({
            .buffer = graph.getBuffer(src),
            .image = graph.getImage(dst),
            .dstLayout = info.layout,
            .dstDimensions = info.dimensions,
            .dstOffsets = info.offsets,
            .dstMipLevel = info.mipLevel,
            .dstLayer = info.layer,
            .dstLayerCount = info.layerCount,
            .size = info.size,
            .srcOffset = info.offset,
        });
    });
    return *this;
}

auto canta::V2::TransferPass::copy(ImageIndex src, BufferIndex dst, ImageCopy info) -> TransferPass& {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, info] (auto& cmd, auto& graph, const auto& push) {
        cmd.copyImageToBuffer({
            .buffer = graph.getBuffer(dst),
            .image = graph.getImage(src),
            .dstLayout = info.layout,
            .dstDimensions = info.dimensions,
            .dstOffsets = info.offsets,
            .dstMipLevel = info.mipLevel,
            .dstLayer = info.layer,
            .dstLayerCount = info.layerCount,
            .size = info.size,
            .srcOffset = info.offset,
        });
    });
    return *this;
}

auto canta::V2::TransferPass::clear(const ImageIndex index, const ClearValue &value) -> TransferPass& {
    addTransferWrite(index);
    pass().setCallback([value, index] (auto& cmd, auto& graph, const auto& push) {
        cmd.clearImage(graph.getImage(index), ImageLayout::TRANSFER_DST, value);
    });
    return *this;
}

auto canta::V2::TransferPass::clear(BufferIndex index, u32 value, u32 offset, u32 size) -> TransferPass& {
    addTransferWrite(index);
    pass().setCallback([index, value, offset, size] (auto& cmd, auto& graph, const auto& push) {
        cmd.clearBuffer(graph.getBuffer(index), value, offset, size);
    });
    return *this;
}

canta::V2::HostPass::HostPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::HostPass::read(const BufferIndex index) -> HostPass & {
    PassBuilder::read(index, Access::HOST_READ, PipelineStage::HOST);
    return *this;
}

auto canta::V2::HostPass::read(const ImageIndex index) -> HostPass & {
    PassBuilder::read(index, Access::HOST_READ, PipelineStage::HOST, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::HostPass::write(const BufferIndex index) -> HostPass & {
    PassBuilder::write(index, Access::HOST_READ, PipelineStage::HOST);
    return *this;
}

auto canta::V2::HostPass::write(const ImageIndex index) -> HostPass & {
    PassBuilder::write(index, Access::HOST_READ, PipelineStage::HOST, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::HostPass::setCallback(const std::function<void(RenderGraph &)> &callback) -> HostPass & {
    pass().setCallback([callback] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        callback(graph);
    });
    return *this;
}

canta::V2::PresentPass::PresentPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

//TODO: will need to handle special. chain timeline semaphore with acquire/present semaphores
auto canta::V2::PresentPass::present(Swapchain* swapchain, const ImageIndex index) -> PresentPass& {
    read(index, Access::MEMORY_READ, PipelineStage::BOTTOM, ImageLayout::PRESENT);
    write(index, Access::MEMORY_READ | Access::MEMORY_WRITE, PipelineStage::BOTTOM, ImageLayout::PRESENT);
    pass().setCallback([swapchain] (auto& cmd, auto& graph, const auto& push) {
        swapchain->present();
    });
    return *this;
}



auto canta::V2::RenderGraph::addBuffer() -> BufferIndex {
    const auto edge = addEdge<BufferIndex>();
    return std::get<BufferIndex>(edge);
}

auto canta::V2::RenderGraph::addImage() -> ImageIndex {
    const auto edge = addEdge<ImageIndex>();
    return std::get<ImageIndex>(edge);
}

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };

auto canta::V2::RenderGraph::alias(BufferIndex index) -> BufferIndex {
    auto& edge = addEdge<BufferIndex>();
    std::visit(overload{
        [index] (BufferIndex& buffer){ buffer.index = index.index; },
            [index] (ImageIndex& image) { static_assert("unreachable"); }
    }, edge);
    return std::get<BufferIndex>(edge);
}

auto canta::V2::RenderGraph::alias(ImageIndex index) -> ImageIndex {
    auto edge = addEdge<ImageIndex>();
    std::visit(overload{
        [index] (BufferIndex& buffer){ static_assert("unreachable"); },
            [index] (ImageIndex& image) { image.index = index.index; },
    }, edge);
    return std::get<ImageIndex>(edge);
}

auto canta::V2::RenderGraph::getBuffer(BufferIndex index) -> BufferHandle {
    return {}; //TODO: actually get buffer not just stub
}

auto canta::V2::RenderGraph::getImage(ImageIndex index) -> ImageHandle {
    return {}; //TODO: actually get image not just stub
}


auto canta::V2::RenderGraph::compute(const std::string_view name) -> ComputePass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::COMPUTE;
    pass._name = name;
    const auto builder = ComputePass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::graphics(const std::string_view name) -> GraphicsPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::GRAPHICS;
    pass._name = name;
    const auto builder = GraphicsPass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::transfer(const std::string_view name) -> TransferPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::TRANSFER;
    pass._name = name;
    const auto builder = TransferPass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::host(const std::string_view name) -> HostPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::HOST;
    pass._name = name;
    const auto builder = HostPass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::present(Swapchain *swapchain, const ImageIndex index) -> PresentPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::PRESENT;
    pass._name = "present_pass";
    auto builder = PresentPass(this, vertexCount() - 1);
    builder.present(swapchain, index);
    return builder;
}

void canta::V2::RenderGraph::setRoot(const BufferIndex index) {
    _rootEdge = index.id;
}

void canta::V2::RenderGraph::setRoot(const ImageIndex index) {
    _rootEdge = index.id;
}

auto canta::V2::RenderGraph::compile() -> std::expected<bool, RenderGraphError> {
    if (_rootEdge < 0) return std::unexpected(RenderGraphError::NO_ROOT);

    const auto sorted = TRY(sort(getEdges()[_rootEdge]).transform_error([] (auto e) {
        switch (e) {
            case ende::graph::Error::IS_CYCLICAL:
                return RenderGraphError::IS_CYCLICAL;
            default:
                return RenderGraphError::IS_CYCLICAL;
        }
    }));

    return true;
}
