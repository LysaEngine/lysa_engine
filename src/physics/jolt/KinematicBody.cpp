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
module lysa.resources.kinematic_body;

import lysa.resources.collision_shape;

namespace lysa {

    KinematicBody::KinematicBody(const std::shared_ptr<CollisionShape>& shape,
                                 const collision_layer  layer) :
        PhysicsBody(shape,
                    layer,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Kinematic) {
    }

    KinematicBody::KinematicBody():
       PhysicsBody(0,
                   JPH::EActivation::Activate,
                   JPH::EMotionType::Kinematic) {
    }

}
