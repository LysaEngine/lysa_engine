/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpasses.transparency_pass;

import lysa.renderers.graphic_pipeline_data;
namespace lysa {

    TransparencyPass::TransparencyPass(
        const RendererConfiguration& config):
        Renderpass{config, "OIT Transparency"},
        materialManager(Context::ctx->res.get<MaterialManager>()) {

        oitPipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        oitPipelineConfig.backStencilOpState = oitPipelineConfig.frontStencilOpState;
        oitPipelineConfig.resources = Context::ctx->vireo->createPipelineResources({
            Context::ctx->globalDescriptorLayout,
            Context::ctx->samplers.getDescriptorLayout(),
            SceneFrameData::sceneDescriptorLayout,
            GraphicPipelineData::pipelineDescriptorLayout,
#ifdef SHADOW_TRANSPARENCY_COLOR_ENABLED
            SceneFrameData::sceneDescriptorLayoutOptional1
#endif
            },
            SceneFrameData::instanceIndexConstantDesc, name);
        oitPipelineConfig.vertexShader = loadShader(VERTEX_SHADER_OIT);
        oitPipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER_OIT);
        oitPipelineConfig.vertexInputLayout =Context::ctx->vireo->createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);

        compositeDescriptorLayout = Context::ctx->vireo->createDescriptorLayout();
        compositeDescriptorLayout->add(BINDING_ACCUM_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        compositeDescriptorLayout->add(BINDING_REVEALAGE_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        compositeDescriptorLayout->build();

        compositePipelineConfig.colorRenderFormats.push_back(config.colorRenderingFormat);
        compositePipelineConfig.resources = Context::ctx->vireo->createPipelineResources({
            Context::ctx->globalDescriptorLayout,
            Context::ctx->samplers.getDescriptorLayout(),
            SceneFrameData::sceneDescriptorLayout,
            compositeDescriptorLayout},
            {}, name);
        compositePipelineConfig.vertexShader = loadShader(VERTEX_SHADER_COMPOSITE);
        compositePipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER_COMPOSITE);
        compositePipeline = Context::ctx->vireo->createGraphicPipeline(compositePipelineConfig, "Transparency OIT Composite");

        framesData.resize(Context::ctx->config.framesInFlight);
        for (auto& frame : framesData) {
            frame.compositeDescriptorSet = Context::ctx->vireo->createDescriptorSet(compositeDescriptorLayout, name);
        }
    }

    void TransparencyPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<unique_id>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!oitPipelines.contains(pipelineId)) {
                const auto& material = materialManager[materials.at(0)];
                std::string fragShaderName = FRAGMENT_SHADER_OIT;
                oitPipelineConfig.cullMode = material.getCullMode();
                oitPipelineConfig.vertexShader = loadShader(VERTEX_SHADER_OIT);
                oitPipelineConfig.fragmentShader = loadShader(fragShaderName);
                oitPipelines[pipelineId] = Context::ctx->vireo->createGraphicPipeline(oitPipelineConfig, "Transparency OIT");
            }
        }
    }

    void TransparencyPass::render(
        vireo::CommandList& commandList,
        const SceneFrameData& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];

        oitRenderingConfig.colorRenderTargets[BINDING_ACCUM_BUFFER].renderTarget = frame.accumBuffer;
        oitRenderingConfig.colorRenderTargets[BINDING_REVEALAGE_BUFFER].renderTarget = frame.revealageBuffer;
        oitRenderingConfig.depthStencilRenderTarget = depthAttachment;

        commandList.barrier(
            {frame.accumBuffer, frame.revealageBuffer},
            vireo::ResourceState::SHADER_READ,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(oitRenderingConfig);
        scene.drawTransparentModels(commandList, oitPipelines);
        commandList.endRendering();
        commandList.barrier(
            {frame.accumBuffer, frame.revealageBuffer},
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);

        frame.compositeDescriptorSet->update(BINDING_ACCUM_BUFFER, frame.accumBuffer->getImage());
        frame.compositeDescriptorSet->update(BINDING_REVEALAGE_BUFFER, frame.revealageBuffer->getImage());
        compositeRenderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;

        commandList.barrier(
             colorAttachment,
             vireo::ResourceState::UNDEFINED,
             vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(compositeRenderingConfig);
        commandList.bindPipeline(compositePipeline);
        commandList.bindDescriptors({
            Context::ctx->globalDescriptorSet,
            Context::ctx->samplers.getDescriptorSet(),
            scene.getDescriptorSet(),
            frame.compositeDescriptorSet
       });
        commandList.draw(3);
        commandList.endRendering();
        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::UNDEFINED);
    }

    void TransparencyPass::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        for (auto& frame : framesData) {
            frame.accumBuffer = Context::ctx->vireo->createRenderTarget(
                oitPipelineConfig.colorRenderFormats[BINDING_ACCUM_BUFFER],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                oitRenderingConfig.colorRenderTargets[BINDING_ACCUM_BUFFER].clearValue);
            frame.revealageBuffer = Context::ctx->vireo->createRenderTarget(
                oitPipelineConfig.colorRenderFormats[BINDING_REVEALAGE_BUFFER],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                oitRenderingConfig.colorRenderTargets[BINDING_REVEALAGE_BUFFER].clearValue);
            commandList->barrier(
              {
                  frame.accumBuffer,
                  frame.revealageBuffer,
                },
              vireo::ResourceState::UNDEFINED,
              vireo::ResourceState::SHADER_READ);
        }
    }

}