/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.animation_track;

namespace lysa {

    /**
    * Returns the interpolated value at the given time (in seconds, from the start of the animation) for a track.
    */
    AnimationTrackKeyValue AnimationTrack::getInterpolatedValue(
        const AnimationLoopMode loopMode,
        const double currentTimeFromStart,
        const bool reverse) const {
        auto value = AnimationTrackKeyValue {
            .ended = (!enabled ||
                (loopMode == AnimationLoopMode::NONE && currentTimeFromStart >= duration) ||
                keyTime.size() < 2),
            .type = type,
        };
        if (value.ended) {
            if (reverse) {
                value.value = keyValue[0];
            } else {
                value.value = keyValue[keyValue.size() - 1];
            }
            return value;
        }

        const auto currentTime = std::fmod(currentTimeFromStart, static_cast<double>(duration));
        value.frameTime = static_cast<float>(currentTime);

        const auto it = std::ranges::lower_bound(keyTime, static_cast<float>(currentTime));
        auto nextIndex = std::distance(keyTime.begin(), it);
        if (nextIndex == 0) {
            if (reverse) {
                value.value = keyValue[keyValue.size() - 1];
            } else {
                value.value = keyValue[0];
            }
            return value;
        }

        auto previousIndex = nextIndex;
        const bool overflow = nextIndex == keyTime.size();

        if (reverse) {
            previousIndex = keyTime.size() - previousIndex;
            nextIndex = keyTime.size() - nextIndex;
        }

        const auto& previousTime = keyTime[previousIndex];
        const auto nextTime = overflow ? duration : keyTime[nextIndex];
        const auto diffTime = nextTime - previousTime;
        const auto interpolationValue = static_cast<float>((currentTime - previousTime) / (
            diffTime > 0 ? diffTime : 1.0f));

        const auto& previousValue = keyValue[previousIndex];
        if (interpolation == AnimationInterpolation::LINEAR) {
            const auto nextValue = overflow ? keyValue[0] : keyValue[nextIndex];
            switch (type) {
            case AnimationType::TRANSLATION:
            case AnimationType::SCALE:
                value.value.xyz = lerp(previousValue.xyz, nextValue.xyz, interpolationValue);
                break;
            case AnimationType::ROTATION: {
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
