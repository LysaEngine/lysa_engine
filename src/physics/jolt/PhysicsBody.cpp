/*
 * Copyright (c) 2024-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
module lysa.resources.physics_body;

import lysa.physics.jolt.engine;

namespace lysa {

    PhysicsBody::PhysicsBody(const std::shared_ptr<CollisionShape>& shape,
                             const collision_layer layer,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType):
        CollisionObject{shape, layer},
        motionType{motionType} {
        this->activationMode = activationMode;
        setShape(shape);
    }

    PhysicsBody::PhysicsBody(const collision_layer layer,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType):
        CollisionObject{layer},
        motionType{motionType} {
        this->activationMode = activationMode;
    }

    void PhysicsBody::setShape(const std::shared_ptr<CollisionShape> &shape) {
        if (this->shape) {
            releaseResources();
        }
        this->shape = shape;
        joltShape = shape->getShapeSettings()->Create().Get();
    }

    void PhysicsBody::createBody(const float3& position, const quaternion& rotation, const float3& scale) {
        const JPH::BodyCreationSettings settings{
            joltShape,
            JPH::RVec3{position.x, position.y, position.z},
            JPH::Quat{rotation.x, rotation.y, rotation.z, rotation.w},
            motionType,
            collisionLayer,
        };
        const auto body = bodyInterface.CreateBody(settings);
        setBodyId(body->GetID());
        if (any(scale != float3{1.0f, 1.0f, 1.0f})) {
            bodyInterface.SetShape(
                bodyId,
                new JPH::ScaledShape(
                    bodyInterface.GetShape(bodyId),
                    JPH::Vec3{scale.x, scale.y, scale.z}),
                true,
                activationMode);
        }
    }

}
