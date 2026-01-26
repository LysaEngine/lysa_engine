/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.resources.light;

import lysa.math;
import lysa.resources;

export namespace lysa {

    /**
     * Light type
     */
    enum class LightType : int8 {
        /** Unknown light type */
        LIGHT_UNKNOWN     = -1,
        /** Directional light */
        LIGHT_DIRECTIONAL = 0,
        /** Omni (point) light */
        LIGHT_OMNI        = 1,
        /** Spot light */
        LIGHT_SPOT        = 2
    };

    /**
    * Light data in GPU memory
    */
    struct LightData {
        /** Light type (Light::LightType) */
        int32 type{-1};
        /** Light range */
        float range{0.0f};
        /** Inner cut-off angle in radians */
        float cutOff{0.0f};
        /** Outer cut-off angle in radians */
        float outerCutOff{0.0f};
        /** Light position in world space */
        float4 position{0.0f};
        /** Light direction */
        float4 direction{0.0f};
        /** Light color (RGB) + Intensity (A) */
        float4 color{1.0f, 1.0f, 1.0f, 1.0f};
        /** Shadow map index */
        int32 mapIndex{-1};
        /** Number of cascades for directional shadows */
        uint32 cascadesCount{0};
        /** Cascade split depths */
        float4 cascadeSplitDepth{0.0f};
        /** Light space matrices for shadows */
        float4x4 lightSpace[6];
    };

    /**
    * A Light resource
    */
    struct Light : UnmanagedResource {
        /** Light type */
        LightType type;
        /** Light color (RGB) */
        float3 color;
        /** Light intensity */
        float intensity;
        /** World space transform */
        float4x4 transform;
        /** Light range */
        float range;
        /** Inner cut-off angle in radians */
        float cutOff;
        /** Outer cut-off angle in radians */
        float outerCutOff;
        /** Whether the light casts shadows */
        bool castShadows;
        /** Resolution of the shadow map */
        uint32 shadowMapSize;
        /** Whether the light is visible/active */
        bool visible{true};
        /** Shadow map near clip distance */
        float shadowMapNearClipDistance{0.01f};
        /** Number of cascades for directional shadow maps */
        uint32 shadowMapCascadesCount{3};
        /** Lambda factor for cascade splitting */
        float shadowMapCascadesSplitLambda{.85f};
        /** Scissors factor for shadow transparency */
        float shadowTransparencyScissors{0.25f};
        /** Color scissors factor for shadow transparency */
        float shadowTransparencyColorScissors{0.75f};

        /**
         * Returns the light position in world space
         */
        float3 getPosition() const { return transform[3].xyz; }

        /**
         * Returns the light front (direction) vector
         */
        float3 getFrontVector() const {  return -normalize(transform[2].xyz); }

        /**
         * Constructor
         * @param type The light type
         * @param color The light color
         * @param intensity The light intensity
         * @param transform The world space transform
         * @param range The light range
         * @param cutOff The inner cut-off angle in radians
         * @param outerCutOff The outer cut-off angle in radians
         * @param castShadows Whether the light casts shadows
         * @param shadowMapSize The resolution of the shadow map
         */
        Light(
            const LightType type,
            const float3& color = {1.0f},
            const float intensity = 1.0f,
            const float4x4& transform = float4x4::identity(),
            const float range = 10.0f,
            const float cutOff = 1.3, // 75
            const float outerCutOff= 1.4, // 80
            const bool castShadows = false,
            const uint32 shadowMapSize = 1024) :
            type(type),
            color(color),
            intensity(intensity),
            transform(transform),
            range(range),
            cutOff(cutOff),
            outerCutOff(outerCutOff),
            castShadows(castShadows),
            shadowMapSize(shadowMapSize) {}

        /**
         * Returns the light data for GPU consumption
         */
        LightData getData() const {
            return {
                .type = static_cast<int32>(type),
                .range = range,
                .cutOff = std::cos(cutOff),
                .outerCutOff = std::cos(outerCutOff),
                .position = float4{getPosition(), 0.0f},
                .direction = float4(getFrontVector(), 0.0f),
                .color = float4(color, intensity),
                .cascadesCount = shadowMapCascadesCount
            };
        }
    };

}
