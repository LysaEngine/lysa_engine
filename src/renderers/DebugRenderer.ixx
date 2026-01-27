/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.h>
#else
// Hack to still compile DebugRenderer when Jolt is compiled without
#define JPH_DEBUG_RENDERER
// Make sure the debug renderer symbols don't get imported or exported
#define JPH_DEBUG_RENDERER_EXPORT
#include <Jolt/Renderer/DebugRenderer.h>
#undef JPH_DEBUG_RENDERER
#undef JPH_DEBUG_RENDERER_EXPORT
#endif
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif
#include <cstdlib>
export module lysa.renderers.debug;

import std;
import vireo;
import lysa.math;
import lysa.renderers.configuration;
import lysa.renderers.vector_3d;

export namespace lysa {


    //! Coloring scheme of collision shapes (only supported by Jolt)
    enum class DebugShapeColor {
        //! Random color per instance
        InstanceColor,
        //! Convex = green, scaled = yellow, compound = orange, mesh = red
        ShapeTypeColor,
        //! Static = grey, keyframed = green, dynamic = random color per instance
        MotionTypeColor,
        //! Static = grey, keyframed = green, dynamic = yellow, sleeping = red
        SleepColor,
    };

    /**
     * Configuration of the in-game debug
     */
    struct DebugConfiguration {
        //! Enable the debug visualization
        bool enabled{false};
        //! If the debug renderer is enabled, display the debug at startup
        bool displayAtStartup{true};
        //! Draw with depth-testing
        bool depthTestEnable{true};
        //! Draw coordinate system (x = red, y = green, z = blue)
        bool drawCoordinateSystem{false};
        //! Coordinate system draw scale
        float coordinateSystemScale{1.0f};
        //! Draw all the rays of the RayCast objects
        bool drawRayCast{false};
        //! Color for the non-colliding rays
        float4 rayCastColor{0.0f, 0.5f, 1.0f, 1.0f};
        //! Color for the colliding rays
        float4 rayCastCollidingColor{0.95f, 0.275f, 0.76f, 1.0f};
        //! Draw the collision shapes of all collision objects
        bool drawShape{true};
        //! Coloring scheme to use for collision shapes
        DebugShapeColor shapeColor{DebugShapeColor::ShapeTypeColor};
        //! Draw a bounding box per collision object
        bool drawBoundingBox{false};
        //! Draw the velocity vectors
        bool drawVelocity{false};
        //! Draw the center of mass for each collision object
        bool drawCenterOfMass{false};
    };

    class DebugRenderer : public Vector3DRenderer
#ifdef PHYSIC_ENGINE_JOLT
        , public JPH::DebugRendererSimple
#endif
    {
    public:
        DebugRenderer(
            const DebugConfiguration& config,
            const RendererConfiguration& renderingConfiguration,
            vireo::ImageFormat outputFormat);

        // void drawRayCasts(const std::shared_ptr<Node>& scene, const float4& rayColor, const float4& collidingRayColor);

        const auto& getConfiguration() const { return config; }

#ifdef PHYSIC_ENGINE_JOLT
        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) override;

        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override {}
#endif

    private:
        const DebugConfiguration& config;
    };
}