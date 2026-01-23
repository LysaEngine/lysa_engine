/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpasses.forward_color_pass;

import lysa.renderers.graphic_pipeline_data;

namespace lysa {
    ForwardColorPass::ForwardColorPass(
        const RendererConfiguration& config):
        Renderpass{config, "Forward Color"},
        materialManager(Context::ctx->res.get<MaterialManager>()) {

        pipelineConfig.colorRenderFormats.push_back(config.colorRenderingFormat); // Color
        if (config.bloomEnabled) {
            pipelineConfig.colorRenderFormats.push_back(config.colorRenderingFormat); // Brightness
            pipelineConfig.colorBlendDesc.push_back({});
            renderingConfig.colorRenderTargets.push_back({ .clear = true });
        }

        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.resources = Context::ctx->vireo->createPipelineResources({
           Context::ctx->globalDescriptorLayout,
           Context::ctx->samplers.getDescriptorLayout(),
           SceneFrameData::sceneDescriptorLayout,
           GraphicPipelineData::pipelineDescriptorLayout,
#ifdef SHADOW_TRANSPARENCY_COLOR_ENABLED
           SceneFrameData::sceneDescriptorLayoutOptional1
#endif
            },
           SceneFrameData::instanceIndexConstantDesc, name);
        pipelineConfig.vertexInputLayout = Context::ctx->vireo->createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);
        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
        renderingConfig.clearDepthStencil = false;

        framesData.resize(Context::ctx->config.framesInFlight);
    }

    void ForwardColorPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<unique_id>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!pipelines.contains(pipelineId)) {
                const auto& material = materialManager[materials.at(0)];
                std::string vertShaderName = DEFAULT_VERTEX_SHADER;
                std::string fragShaderName = config.bloomEnabled ? DEFAULT_FRAGMENT_BLOOM_SHADER : DEFAULT_FRAGMENT_SHADER;
                if (material.getType() == Material::SHADER) {
                    const auto& shaderMaterial = dynamic_cast<const ShaderMaterial&>(material);
                    if (!shaderMaterial.getVertFileName().empty()) {
                        vertShaderName = shaderMaterial.getVertFileName();
                    }
                    if (!shaderMaterial.getFragFileName().empty()) {
                        fragShaderName = shaderMaterial.getFragFileName();
                    }
                }
                pipelineConfig.cullMode = material.getCullMode();
                pipelineConfig.vertexShader = loadShader(vertShaderName);
                pipelineConfig.fragmentShader = loadShader(fragShaderName);
                pipelineConfig.msaa = config.msaa;
                pipelines[pipelineId] = Context::ctx->vireo->createGraphicPipeline(pipelineConfig, vertShaderName + "+" + fragShaderName + ":" + std::to_string(pipelineId));
            }
        }
    }

    void ForwardColorPass::render(
        vireo::CommandList& commandList,
        const SceneFrameData& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const std::shared_ptr<vireo::RenderTarget>& multisampledDepthAttachment,
        const bool clearAttachment,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];renderingConfig.multisampledDepthStencilRenderTarget = multisampledDepthAttachment;

        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        if (config.bloomEnabled) {
            renderingConfig.colorRenderTargets[1].renderTarget = frame.brightnessBuffer;
        }
        renderingConfig.depthStencilRenderTarget = depthAttachment;
        if (config.msaa != vireo::MSAA::NONE) {
            renderingConfig.colorRenderTargets[0].multisampledRenderTarget = frame.multisampledColorAttachment;
            commandList.barrier(
                frame.multisampledColorAttachment,
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::RENDER_TARGET_COLOR);
        }

        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.barrier(
           frame.brightnessBuffer,
           vireo::ResourceState::SHADER_READ,
           vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(renderingConfig);
        scene.drawOpaquesModels(
            commandList,
            pipelines);
        commandList.endRendering();
        commandList.barrier(
            frame.brightnessBuffer,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::UNDEFINED);
        if (config.msaa != vireo::MSAA::NONE) {
            commandList.barrier(
                frame.multisampledColorAttachment,
                vireo::ResourceState::RENDER_TARGET_COLOR,
                vireo::ResourceState::UNDEFINED);
        }
    }

    void ForwardColorPass::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        for (auto& frame : framesData) {
            if (config.bloomEnabled) {
                frame.brightnessBuffer = Context::ctx->vireo->createRenderTarget(
                    pipelineConfig.colorRenderFormats[1],
                    extent.width,extent.height,
                    vireo::RenderTargetType::COLOR,
                    renderingConfig.colorRenderTargets[1].clearValue,
                    1,
                    vireo::MSAA::NONE,
                    "Brightness");
            } else {
                frame.brightnessBuffer = Context::ctx->vireo->createRenderTarget(
                    pipelineConfig.colorRenderFormats[0],
                    1, 1,
                    vireo::RenderTargetType::COLOR,
                    renderingConfig.colorRenderTargets[0].clearValue);
            }
            commandList->barrier(
               frame.brightnessBuffer,
               vireo::ResourceState::UNDEFINED,
               vireo::ResourceState::SHADER_READ);
            if (config.msaa != vireo::MSAA::NONE) {
                frame.multisampledColorAttachment = Context::ctx->vireo->createRenderTarget(
                  config.colorRenderingFormat,
                  extent.width, extent.height,
                  vireo::RenderTargetType::COLOR,
                  {config.clearColor.r, config.clearColor.g, config.clearColor.b, 1.0f},
                  1,
                  config.msaa,
                  "MSAA color attachment");
            }
        }
    }
}