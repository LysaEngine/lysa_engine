/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation_library;

import std;
import lysa.resources;
import lysa.resources.animation;

export namespace lysa {

    /**
     * Container for \ref Animation resources.
     */
    template<typename T_3DOBJECT>
    class AnimationLibrary : public UnmanagedResource {
    public:
        AnimationLibrary() = default;

        AnimationLibrary(const AnimationLibrary& animationLibrary) :
            defaultAnimation(animationLibrary.defaultAnimation) {
            for (auto& anim : animationLibrary.animations) {
                animations.insert(std::make_pair(anim.first, std::make_shared<Animation<T_3DOBJECT>>(*anim.second)));
            }
        }

        /**
         * Adds the \ref Animation to the library, accessible by the key name.
         */
        void addAnimation(const std::string& keyName, const std::shared_ptr<Animation<T_3DOBJECT>>& animation) {
            if (animations.empty()) {
                defaultAnimation = keyName;
            }
            animations[keyName] = animation;
        }

        /**
         * Returns the \ref Animation with the key name.
         */
        auto getAnimation(const std::string& keyName) const { return animations.at(keyName); }

        /**
         * Returns `true` if the library stores an \ref Animation with name as the key.
         */
        auto hasAnimation(const std::string& keyName) const { return animations.contains(keyName); }

        /**
         * Returns the name of the default animation
         */
        const auto& getDefaultAnimationName() const { return defaultAnimation; }

        void reset() {
            for (auto& animation : animations | std::views::values) {
                animation->reset();
            }
        }

    private:
        std::string defaultAnimation;
        std::map<std::string, std::shared_ptr<Animation<T_3DOBJECT>>> animations;
    };

}
