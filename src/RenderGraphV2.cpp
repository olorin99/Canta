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

auto canta::V2::ComputePass::dispatchThreads(u32 x, u32 y, u32 z) -> ComputePass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchThreads(x, y, z);
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
