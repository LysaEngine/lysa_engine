/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation;

import lysa.assets_pack;
import lysa.math;
import lysa.resources;

export namespace lysa {

    /**
     * Holds data that can be used to animate anything.
     */
    class Animation : public UnmanagedResource {
    public:

        /**
         * An animation track
         */
        struct Track {
            AnimationType           type;
            AnimationInterpolation  interpolation{AnimationInterpolation::LINEAR};
            bool                    enabled{true};
            float                   duration{0.0f};
            std::vector<float>      keyTime;
            std::vector<float4>     keyValue;
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

}
