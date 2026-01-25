/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation;

import lysa.exception;
import lysa.math;
import lysa.resources;
import lysa.resources.animation_track;

export namespace lysa {

    /**
     * Holds data that can be used to animate anything.
     */
    class Animation : public UnmanagedResource {
    public:

        /**
         * Creates an Animation
         * @param tracksCount number of tracks to allocate
         * @param name
         */
        Animation(uint32 tracksCount, const std::string &name);

        Animation(const Animation& anim);

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
        auto& getTrack(const uint32 index) { return tracks[index]; }

        const auto& getName() const { return name; }

        const auto& getTracks() const { return tracks; }

    private:
        AnimationLoopMode loopMode{AnimationLoopMode::NONE};
        std::vector<AnimationTrack> tracks;
        const std::string name;
    };


}
