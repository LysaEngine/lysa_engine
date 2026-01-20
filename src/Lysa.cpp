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

    _LysaInit::_LysaInit(const LoggingConfiguration &loggingConfiguration) {
        if constexpr (Log::isLoggingEnabled()) Log::init(loggingConfiguration);
#ifdef USE_SDL3
        SDL_Init(SDL_INIT_VIDEO);
#endif
    }

    _LysaInit::~_LysaInit() {
#ifdef USE_SDL3
        SDL_Quit();
#endif
        if constexpr (Log::isLoggingEnabled()) Log::shutdown();
    }

    Lysa::Lysa(const ContextConfiguration& config, const LoggingConfiguration &loggingConfiguration) :
        _LysaInit(loggingConfiguration),
        ctx(config),
        fixedDeltaTime(config.deltaTime),
        imageManager(ctx, config.resourcesCapacity.images),
        materialManager(ctx, config.resourcesCapacity.material),
        meshManager(ctx,
                    config.resourcesCapacity.meshes,
                    config.resourcesCapacity.vertices,
                    config.resourcesCapacity.indices,
                    config.resourcesCapacity.surfaces),
        globalDescriptors(ctx)
    {
        ctx.globalDescriptorLayout = globalDescriptors.getDescriptorLayout();
        ctx.globalDescriptorSet = globalDescriptors.getDescriptorSet();
        SceneFrameData::createDescriptorLayouts(ctx);
    }

    Lysa::~Lysa() {
        ctx.graphicQueue->waitIdle();
        SceneFrameData::destroyDescriptorLayouts();
        Renderpass::destroyShaderModules();
        FrustumCulling::cleanup();
    }

    void Lysa::uploadData() {
        if (ctx.samplers.isUpdateNeeded()) {
            ctx.samplers.update();
        }
        materialManager.flush();
        meshManager.flush();
        globalDescriptors.update();
    }

    void Lysa::run() {
        while (!ctx.exit) {
            uploadData();
            ctx.defer._process();
            ctx.threads._process();
            processPlatformEvents();
            ctx.events._process();
            uploadData();

            // https://gafferongames.com/post/fix_your_timestep/
            const auto newTime = std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            auto frameTime = newTime - currentTime;
            if (frameTime > 0.25) frameTime = 0.25; // Note: Max frame time to avoid spiral of death

            // Display the average FPS in the log
            if (ctx.config.displayFPS) {
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
                ctx.events.fire(event);
                accumulator -= fixedDeltaTime;
            }
            auto event = Event{MainLoopEvent::PROCESS,accumulator / fixedDeltaTime };
            ctx.events.fire(event);
        }
        ctx.defer._process();
        auto event = Event{ MainLoopEvent::QUIT };
        ctx.events.fire(event);
        ctx.graphicQueue->waitIdle();
    }

}