/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.resources.camera;

import lysa.math;
import lysa.resources;

export namespace lysa {

    /**
     * Represents a camera in the 3D scene.
     */
    struct Camera : UnmanagedResource {
        /** World-space transformation matrix (world-from-local). */
        float4x4 transform{float4x4::identity()};
        /** Projection matrix (clip-from-view). */
        float4x4 projection{};
        /** Near clipping plane distance. */
        float near{0.1};
        /** Far clipping plane distance. */
        float far{100.0};

        Camera() = default;

        Camera(const Camera& camera) :
           transform(camera.transform),
           projection(camera.projection),
           near(camera.near),
           far(camera.far) {}

        /**
         * Constructs a new Camera object.
         * @param transform World-space transformation matrix.
         * @param projection Projection matrix.
         * @param near Near clipping plane distance.
         * @param far Far clipping plane distance.
         */
        Camera(const float4x4& transform, const float4x4& projection, const float near, const float far) :
            transform(transform),
            projection(projection),
            near(near),
            far(far) {}
    };

}

