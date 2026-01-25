/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation_track;

import lysa.exception;
import lysa.math;
import lysa.resources;

export namespace lysa {

    /**
    * Animation type for an animation track
    */
    enum class AnimationType : uint8 {
        /**
         * The values are the translation along the X, Y, and Z axes.
         */
        TRANSLATION = 1,
        /**
         * The values are a quaternion in the order x, y, z, w where w is the scalar.
         */
        ROTATION = 2,
        /**
         * The values are scaling factors along the X, Y, and Z axes.
         */
        SCALE = 3,
        // Weights = 4,
    };

    /**
     * Interpolation type to apply when calculating animation values
     */
    enum class AnimationInterpolation : uint8 {
        /**
         * The animated values are linearly interpolated between keyframes..
         */
        LINEAR = 0,
        /**
         * The animated values remain constant to the output of the first keyframe, until the next
         * keyframe.
         */
        STEP = 1,
        /**
         * The animation’s interpolation is computed using a cubic spline with specified tangents.
         */
        // CUBIC = 2,
    };

    /**
     * Animation loop mode
     */
    enum class AnimationLoopMode : uint8 {
        //! No loop (default)
        NONE    = 0,
        //! Restart from the start of the track
        LINEAR  = 1,
    };

    struct AnimationTrackKeyValue {
        //! `true` if we reach the end of the track
        bool           ended;
        //! corresponding time from the start of the track
        float          frameTime;
        //! animation type
        AnimationType  type;
        //! interpolated value
        float4          value;
    };

    /**
     * An animation track
     */
    struct AnimationTrack : UnmanagedResource {
        AnimationType          type;
        AnimationInterpolation interpolation{AnimationInterpolation::LINEAR};
        bool                   enabled{true};
        float                  duration{0.0f};
        std::vector<float>     keyTime;
        std::vector<float4>    keyValue;
        std::string            path;

        AnimationTrack() = default;

        AnimationTrack(const AnimationTrack& track) :
            type(track.type),
            interpolation(track.interpolation),
            enabled(track.enabled),
            duration(track.duration),
            keyTime(track.keyTime),
            keyValue(track.keyValue),
            path(track.path) {}

        /**
        * Returns the interpolated value at the given time (in seconds, from the start of the animation) for a track.
        */
        AnimationTrackKeyValue getInterpolatedValue(
            AnimationLoopMode loopMode,
            double currentTimeFromStart,
            bool reverse) const;
    };


}
