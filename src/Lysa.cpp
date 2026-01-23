/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#ifdef USE_SDL3
#include <SDL3/SDL.h>
#endif
module lysa;
import lysa.renderers.scene_frame_data;

namespace lysa {

    _LysaInit::_LysaInit(const ContextConfiguration& config, const LoggingConfiguration &loggingConfiguration) {
        if constexpr (Log::isLoggingEnabled()) Log::init(loggingConfiguration);
        assert([&]{ return Context::ctx == nullptr; }, "Context already initialized");
#ifdef USE_SDL3
        SDL_Init(SDL_INIT_VIDEO);
#endif
        Context::ctx = std::make_unique<Context>(config);
    }

    _LysaInit::~_LysaInit() {
        Context::ctx.reset();
#ifdef USE_SDL3
        SDL_Quit();
#endif
        if constexpr (Log::isLoggingEnabled()) Log::shutdown();
    }

    Lysa::Lysa(const ContextConfiguration& config, const LoggingConfiguration &loggingConfiguration) :
        _LysaInit(config, loggingConfiguration),
        fixedDeltaTime(config.deltaTime),
        imageManager(config.resourcesCapacity.images),
        materialManager(config.resourcesCapacity.material),
        meshManager(
            config.resourcesCapacity.meshes,
            config.resourcesCapacity.vertices,
            config.resourcesCapacity.indices,
            config.resourcesCapacity.surfaces) {
        Context::ctx->globalDescriptorLayout = globalDescriptors.getDescriptorLayout();
        Context::ctx->globalDescriptorSet = globalDescriptors.getDescriptorSet();
        SceneFrameData::createDescriptorLayouts();
    }

    Lysa::~Lysa() {
        Context::ctx->graphicQueue->waitIdle();
        SceneFrameData::destroyDescriptorLayouts();
        Renderpass::destroyShaderModules();
        FrustumCulling::cleanup();
    }

    void Lysa::uploadData() {
        if (Context::ctx->samplers.isUpdateNeeded()) {
            Context::ctx->samplers.update();
        }
        materialManager.flush();
        meshManager.flush();
        globalDescriptors.update();
    }

    void Lysa::run() {
        while (!Context::ctx->exit) {
            uploadData();
            Context::ctx->defer._process();
            Context::ctx->threads._process();
            processPlatformEvents();
            Context::ctx->events._process();
            uploadData();

            // https://gafferongames.com/post/fix_your_timestep/
            const auto newTime = std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            auto frameTime = newTime - currentTime;
            if (frameTime > 0.25) frameTime = 0.25; // Note: Max frame time to avoid spiral of death

            // Display the average FPS in the log
            if (Context::ctx->config.displayFPS) {
                elapsedSeconds += static_cast<float>(frameTime);
                frameCount++;
                if (elapsedSeconds >= 1.0) {
                    fps            = static_cast<uint32_t>(frameCount / elapsedSeconds);
                    frameCount     = 0;
                    elapsedSeconds = 0;
                    Log::info("FPS ", fps);
                }
            }

            currentTime = newTime;
            accumulator += frameTime;
            while (accumulator >= fixedDeltaTime) {
                auto event = Event{MainLoopEvent::PHYSICS_PROCESS,fixedDeltaTime };
                Context::ctx->events.fire(event);
                accumulator -= fixedDeltaTime;
            }
            auto event = Event{MainLoopEvent::PROCESS,accumulator / fixedDeltaTime };
            Context::ctx->events.fire(event);
        }
        Context::ctx->defer._process();
        auto event = Event{ MainLoopEvent::QUIT };
        Context::ctx->events.fire(event);
        Context::ctx->graphicQueue->waitIdle();
    }

}