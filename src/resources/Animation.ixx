/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation;

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

    /**
     * Holds data that can be used to animate anything.
     */
    template<typename T_3DOBJECT>
    class Animation : public UnmanagedResource {
    public:
        /**
         * An animation track
         */
        struct Track {
            AnimationType          type;
            AnimationInterpolation interpolation{AnimationInterpolation::LINEAR};
            bool                   enabled{true};
            float                  duration{0.0f};
            std::vector<float>     keyTime;
            std::vector<float4>    keyValue;
            T_3DOBJECT*            target{nullptr};
        };

        /**
         * Values returned from getInterpolatedValue()
         */
        struct TrackKeyValue {
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
         * Creates an Animation
         * @param tracksCount number of tracks to allocate
         */
        Animation(uint32 tracksCount, const std::string &name);

        /**
         * Sets the looping mode
         */
        void setLoopMode(const AnimationLoopMode mode) { loopMode = mode; }

        /**
         * Returns the looping mode
         */
        auto getLoopMode() const { return loopMode; }

        /**
         * Returns the number of tracks
         */
        auto getTracksCount() const { return tracks.size(); }

        /**
         * Returns a given track
         */
        auto& getTrack(const uint32 index) { return tracks.at(index); }

        /**
         * Returns the interpolated value at the given time (in seconds, from the start of the animation) for a track.
         */
        TrackKeyValue getInterpolatedValue(uint32 trackIndex, double currentTimeFromStart, bool reverse=false) const;

        const auto& getName() const { return name; }

    private:
        AnimationLoopMode loopMode{AnimationLoopMode::NONE};
        std::vector<Track> tracks;
        const std::string name;
    };


    template<typename T_3DOBJECT>
    Animation<T_3DOBJECT>::Animation(const uint32 tracksCount, const std::string &name): name {name} {
        tracks.resize(tracksCount);
    }

    template<typename T_3DOBJECT>
    Animation<T_3DOBJECT>::TrackKeyValue Animation<T_3DOBJECT>::getInterpolatedValue(
        const uint32 trackIndex,
        const double currentTimeFromStart,
        const bool reverse) const {
        assert([&]{ return trackIndex < tracks.size(); }, "Track index out of range");
        const auto& track = tracks[trackIndex];
        auto value = TrackKeyValue{
            .ended = (!track.enabled ||
                    (loopMode == AnimationLoopMode::NONE && currentTimeFromStart >= track.duration) ||
                    track.keyTime.size() < 2),
            .type = track.type,
        };
        if (value.ended) {
            if (reverse) {
                value.value = track.keyValue[0];
            } else {
                value.value = track.keyValue[track.keyValue.size() - 1];
            }
            return value;
        }

        const auto currentTime = std::fmod(currentTimeFromStart, static_cast<double>(track.duration));
        value.frameTime = static_cast<float>(currentTime);

        const auto it = std::ranges::lower_bound(track.keyTime, static_cast<float>(currentTime));
        auto nextIndex = std::distance(track.keyTime.begin(), it);
        if (nextIndex == 0) {
            if (reverse) {
                value.value = track.keyValue[track.keyValue.size() - 1];
            } else {
                value.value = track.keyValue[0];
            }
            return value;
        }

        auto previousIndex = nextIndex;
        const bool overflow = nextIndex == track.keyTime.size();

        if (reverse) {
            previousIndex = track.keyTime.size() - previousIndex;
            nextIndex = track.keyTime.size() - nextIndex;
        }

        const auto& previousTime = track.keyTime[previousIndex];
        const auto nextTime = overflow ? track.duration : track.keyTime[nextIndex];
        const auto diffTime = nextTime - previousTime;
        const auto interpolationValue = static_cast<float>((currentTime - previousTime) / (diffTime > 0 ? diffTime : 1.0f));

        const auto& previousValue = track.keyValue[previousIndex];
        if (track.interpolation == AnimationInterpolation::LINEAR) {
            const auto nextValue = overflow ? track.keyValue[0] : track.keyValue[nextIndex];
            switch (track.type) {
                case AnimationType::TRANSLATION:
                case AnimationType::SCALE:
                    value.value.xyz = lerp(previousValue.xyz, nextValue.xyz, interpolationValue);
                    break;
                case AnimationType::ROTATION:{
                    const auto prev = quaternion{previousValue};
                    const auto next = quaternion{nextValue};
                    value.value = lerp(prev, next, interpolationValue).xyzw;
                    break;
                }
            }
        } else {
            // STEP
            value.value = previousValue;
        }
        return value;
    }


}
