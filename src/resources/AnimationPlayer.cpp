/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.animation_player;

import lysa.context;
import lysa.event;

namespace lysa {

    void AnimationPlayer::seek(const float duration) {
        const auto animation = getAnimation();
        for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
            const auto& value = animation->getInterpolatedValue(
                       trackIndex,
                       duration,
                       false);
            currentTracksState[trackIndex] = value.frameTime;
            apply(value);
        }
    }

    void AnimationPlayer::setCurrentLibrary(const std::string &name) {
        if (libraries.contains(name)) {
            currentLibrary = name;
            setCurrentAnimation(libraries[currentLibrary]->getDefault());
        }
    }

    void AnimationPlayer::setCurrentAnimation(const std::string &name) {
        if (libraries[currentLibrary]->has(name)) {
            currentAnimation = name;
            if (currentTracksState.size() != getAnimation()->getTracksCount()) {
                currentTracksState.resize(getAnimation()->getTracksCount());
                lastTracksState.resize(getAnimation()->getTracksCount());
                std::ranges::fill(lastTracksState, 0.0f);
            }
        }
    }

    void AnimationPlayer::play(const std::string &name) {
        if (playing) { return; }
        if (name.empty()) {
            setCurrentAnimation(libraries[currentLibrary]->getDefault());
        } else {
            setCurrentAnimation(name);
        }
        starting = true;
        reverse = false;
    }

    void AnimationPlayer::playBackwards(const std::string &name) {
        if (playing) { return; }
        play(name);
        reverse = true;
    }

    void AnimationPlayer::stop(const bool keepState) {
        if (!playing) { return; }
        playing = false;
        if (keepState) {
            lastTracksState = currentTracksState;
        } else {
            std::ranges::fill(lastTracksState, 0.0f);
        }
    }

    std::shared_ptr<Animation> AnimationPlayer::getAnimation() {
        return libraries[currentLibrary]->get(currentAnimation);
    }

    void AnimationPlayer::update() {
        if (starting) {
            startTime = std::chrono::steady_clock::now();
            playing = true;
            starting = false;
            auto params = Playback{.animationName = currentAnimation};
            auto event = Event{AnimationPlayerEvent::START, params, id};
            ctx().events.fire(event);
        } else if (!playing) {
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        const auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count()) / 1000.0;
        const auto animation = getAnimation();
        if (animation) {
            for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
                const auto& value = animation->getInterpolatedValue(
                    trackIndex,
                    duration + lastTracksState[trackIndex],
                    reverse);
                currentTracksState[trackIndex] = value.frameTime;
                if (value.ended) {
                    stop();
                    auto params = Playback{.animationName = currentAnimation};
                    auto event = Event{AnimationPlayerEvent::FINISH, params, id};
                    ctx().events.fire(event);
                } else {
                    apply(value);
                }
            }
        }
    }

}
