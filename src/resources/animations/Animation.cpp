/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.animation;

namespace lysa {

    Animation::Animation(uint32 tracksCount, const std::string& name) :
        name{name} {
        tracks.resize(tracksCount);
    }

    Animation::Animation(const Animation& anim) :
        loopMode(anim.loopMode),
        name(anim.name) {
        tracks.reserve(tracks.size());
        for (auto& track : anim.tracks) {
            tracks.emplace_back(track);
        }
    }

}
