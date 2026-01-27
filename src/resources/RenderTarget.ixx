/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.resources.render_target;

import vireo;
import lysa.event;
import lysa.input_event;
import lysa.math;
import lysa.renderers.configuration;
import lysa.renderers.graphic_pipeline_data;
import lysa.renderers.renderer;
import lysa.renderers.vector_2d;
import lysa.renderers.vector_3d;
import lysa.resources;
import lysa.resources.render_view;
import lysa.resources.camera;
import lysa.resources.scene;

export namespace lysa {

    /**
     * Render target configuration
     */
    struct RenderTargetConfiguration {
        /** Presentation mode */
        vireo::PresentMode presentMode{vireo::PresentMode::IMMEDIATE};
        /** Configuration for the rendering path of the target */
        RendererConfiguration rendererConfiguration;
    };

    /**
    * Render target events data
    */
    struct RenderTargetEvent {
        /** The render target has been paused */
        static constexpr auto PAUSED{"RENDERING_TARGET_PAUSED"};
        /** The render target has been resumed */
        static constexpr auto RESUMED{"RENDERING_TARGET_RESUMED"};
        /** The render target has been resized */
        static constexpr auto RESIZED{"RENDERING_TARGET_RESIZED"};
    };

    /**
     * Render target for displaying the scene
     */
    class RenderTarget : public UniqueResource {
    public:
        /**
         * Creates a RenderTarget
         * @param configuration The configuration for the render target
         * @param renderingWindowHandle The handle to the platform window
         */
        RenderTarget(
            const RenderTargetConfiguration& configuration,
            vireo::PlatformWindowHandle renderingWindowHandle);

        ~RenderTarget() override;

        /** Renders the scene into the target */
        void render();

        /**
         * Returns `true` if the rendering is paused
         */
        auto isPaused() const { return paused; }

        /**
         * Pauses or resumes the rendering
         */
        void setPause(bool pause);

        /**
         * Returns the width in pixels of the render target
         */
        float getWidth() const { return static_cast<float>(swapChain->getExtent().width); }

        /**
         * Returns the height in pixels of the render target
         */
        float getHeight() const { return static_cast<float>(swapChain->getExtent().height); }

        /**
         * Returns the aspect ratio ((width / height) of the render target
         */
        float getAspectRatio() const { return swapChain->getAspectRatio(); }

        /**
         * Returns the number of frames in flight
         */
        uint32 getFramesInFlight() const { return swapChain->getFramesInFlight(); }

        /**
         * Returns the current frame index
         */
        uint32 getCurrentFrameIndex() const { return swapChain->getCurrentFrameIndex(); }

        /**
         * Returns the extent of the swap chain
         */
        const vireo::Extent& getExtent() const { return swapChain->getExtent(); }

        /**
         * Returns the image format of the swap chain
         */
        vireo::ImageFormat getImageFormat() const { return swapChain->getFormat(); }

        /** Waits for the GPU to finish all work on this target */
        void waitIdle() const { swapChain->waitIdle(); }

        /**
         * Adds a view to the render target
         */
        void addView(RenderView& view);

        /**
         * Updates a view in the render target
         */
        void updateView(RenderView& view);

        /**
         * Removes a view from the render target
         */
        void removeView(const RenderView& view);

        /**
         * Updates the pipelines
         */
        void updatePipelines(const std::unordered_map<pipeline_id, std::vector<unique_id>>& pipelineIds) const;

        /** Resizes the render target */
        void resize();

        /**
         * Returns the renderer configuration
         */
        const RendererConfiguration& getRendererConfiguration() const { return rendererConfiguration; }

        /**
         * Adds a 3D vector renderer
         */
        void addRenderer(Vector3DRenderer& vector3DRenderer) {
            vector3DRenderers.push_back(&vector3DRenderer);
        }

        /**
         * Returns the scene renderer
         */
        Renderer& getRenderer() const { return *renderer; }

    private:
        /* Per-frame data */
        struct FrameData {
            /* Fence signaled when the frame's work has completed on GPU. */
            std::shared_ptr<vireo::Fence> inFlightFence;
            /* Command allocator for this frame (resets between frames). */
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            /* Semaphore signaled when data update stage is finished. */
            std::shared_ptr<vireo::Semaphore> updateSemaphore;
            /* Semaphore signaled when compute stage is finished. */
            std::shared_ptr<vireo::Semaphore> computeSemaphore;
            /* Semaphore signaled when pre‑render stage is finished. */
            std::shared_ptr<vireo::Semaphore> prepareSemaphore;
            /* Command list used for data update workloads. */
            std::shared_ptr<vireo::CommandList> updateCommandList;
            /* Command list used for compute workloads. */
            std::shared_ptr<vireo::CommandList> computeCommandList;
            /* Command list used for rendering into the swap chain. */
            std::shared_ptr<vireo::CommandList> prepareCommandList;
            /* Command list used for rendering into the swap chain. */
            std::shared_ptr<vireo::CommandList> renderCommandList;
        };

        /* The renderer configuration */
        const RendererConfiguration rendererConfiguration;
        /* Set to true to pause the rendering in this target */
        bool paused{false};
        /* Array of per‑frame resource bundles (size = frames in flight). */
        std::vector<FrameData> framesData;
        /* Swap chain presenting the render target in memory. */
        std::shared_ptr<vireo::SwapChain> swapChain{nullptr};
        /* Views to render in this target */
        std::list<RenderView> views;
        /* Scene renderer used to draw attached views. */
        std::unique_ptr<Renderer> renderer;
        /* Additional 3D Vector renderers */
        std::vector<Vector3DRenderer*> vector3DRenderers;
        /* Protect views to be modifies when render() is called */
        std::mutex viewsMutex;
        /* The main viewport */
        vireo::Viewport mainViewport;
        /* The main scissors */
        vireo::Rect mainScissors;
    };

}

