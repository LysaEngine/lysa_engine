/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation_player;

import std;
import lysa.math;
import lysa.resources;
import lysa.resources.animation;
import lysa.resources.animation_library;

export namespace lysa {

    /**
    * Render target events data
    */
    struct AnimationPlayerEvent {
        //! Fired when an animation began playing
        static constexpr auto START{"ANIMATION_PLAYER_START"};
        //! Fired when an animation stops playing
        static constexpr auto FINISH{"ANIMATION_PLAYER_FINISH"};
    };

    /**
     * %A node used for animation playback.
     */
    class AnimationPlayer : public UnmanagedResource {
    public:
        /**
         * Signal playback parameters
         */
        struct Playback {
            //! The animation name
            std::string animationName;
        };

        AnimationPlayer() = default;

        AnimationPlayer(const AnimationPlayer& animationPlayer);

        /**
         * Returns the current library name
         */
        const auto& getCurrentLibrary() const { return currentLibrary; }

        /**
         * Returns the current animation name
         */
        const auto& getCurrentAnimation() const { return currentAnimation; }

        /**
         * Sets the current library name
         */
        void setCurrentLibrary(const std::string &name);

        /**
         * Sets the current animation name (does not start the animation, only useful with auto-starting)
         */
        void setCurrentAnimation(const std::string &name);

        /**
         * Adds a library accessible by the name.
         */
        auto add(const std::string& name, const std::shared_ptr<AnimationLibrary>& library) { libraries[name] = library; }

        /**
         * Returns the current animation, if any
         */
        std::shared_ptr<Animation> getAnimation();

        /**
         * Returns the current animation library, if any
         */
        auto getLibrary() { return libraries[currentLibrary]; }

        /**
         * Starts an animation by its name
         */
        void play(const std::string &name = "");

        /**
         *
         * Seeks the animation to the point in time (in seconds).
         */
        void seek(float duration);

        /**
         * Starts an animation by its name, playing it backwards
         */
        void playBackwards(const std::string &name = "");

        /**
         * Stops the currently playing animation
         */
        void stop(bool keepState = false);

        /**
         * Returns `true` if the animation is currently playing
         */
        auto isPlaying() const { return playing; }

        /**
         * Sets the auto start property.
         */
        auto setAutoStart(const bool autoStart) { this->autoStart = autoStart; }

    protected:
        bool autoStart{false};
        bool playing{false};
        bool starting{false};
        bool reverse{false};
        float3 initialPosition{0.0f};
        quaternion initialRotation{quaternion::identity()};
        float3 initialScale{1.0f};
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        std::string currentLibrary;
        std::string currentAnimation;
        std::vector<float> currentTracksState;
        std::vector<float> lastTracksState;
        std::map<std::string, std::shared_ptr<AnimationLibrary>> libraries;

        virtual void apply(const Animation::TrackKeyValue&) const {};

        virtual void update();

    };

}
