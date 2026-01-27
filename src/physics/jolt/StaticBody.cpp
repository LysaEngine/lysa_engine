/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
module lysa.resources.static_body;

namespace lysa {

    StaticBody::StaticBody(const std::shared_ptr<CollisionShape>& shape,
                           const collision_layer layer):
        PhysicsBody(shape,
                    layer,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static) {
    }

    StaticBody::StaticBody(const collision_layer layer):
        PhysicsBody(layer,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static) {
    }

    StaticBody::StaticBody():
        PhysicsBody(0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static) {
    }

}
