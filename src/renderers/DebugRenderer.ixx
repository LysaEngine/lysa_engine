/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif
export module lysa.renderers.debug_renderer;

import std;
import vireo;
import lysa.math;
import lysa.renderers.configuration;
import lysa.renderers.vector_3d;

export namespace lysa {

    /**
     * Renderer for debugging information
     */
    class DebugRenderer : public Vector3DRenderer
#ifdef PHYSIC_ENGINE_JOLT
        , public JPH::DebugRendererSimple
#endif
    {
    public:
        /**
         * Constructor
         * @param config Debug configuration
         * @param renderingConfiguration Rendering configuration
         * @param outputFormat Output image format
         */
        DebugRenderer(
            const DebugConfiguration& config,
            const RendererConfiguration& renderingConfiguration,
            vireo::ImageFormat outputFormat);

        // void drawRayCasts(const std::shared_ptr<Node>& scene, const float4& rayColor, const float4& collidingRayColor);

        /**
         * Get the debug configuration
         */
        const auto& getConfiguration() const { return config; }

#ifdef PHYSIC_ENGINE_JOLT
        /**
         * Draw a line (Jolt callback)
         * @param inFrom Start point
         * @param inTo End point
         * @param inColor Line color
         */
        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

        /**
         * Draw a triangle (Jolt callback)
         * @param inV1 Vertex 1
         * @param inV2 Vertex 2
         * @param inV3 Vertex 3
         * @param inColor Triangle color
         * @param inCastShadow Whether to cast shadow
         */
        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) override;

        /**
         * Draw 3D text (Jolt callback, currently not implemented)
         * @param inPosition Text position
         * @param inString Text string
         * @param inColor Text color
         * @param inHeight Text height
         */
        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override {}
#endif

    private:
        const DebugConfiguration& config;
    };
}