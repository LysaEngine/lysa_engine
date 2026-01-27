/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
module lysa.resources.collision_area;

namespace lysa {

    void CollisionArea::setShape(const std::shared_ptr<CollisionShape> &shape) {
        if (this->shape) {
            releaseResources();
        }
        this->shape = shape;
        joltShape = shape->getShapeSettings()->Create().Get();
    }

    void CollisionArea::createBody(const float3& position, const quaternion& rotation) {
        JPH::BodyCreationSettings settings{
            joltShape,
            JPH::RVec3{position.x, position.y, position.z},
            JPH::Quat{rotation.x, rotation.y, rotation.z, rotation.w},
            JPH::EMotionType::Dynamic,
            collisionLayer,
        };
        settings.mIsSensor = true;
        settings.mUseManifoldReduction = true;
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
        settings.mMassPropertiesOverride = JPH::MassProperties{.mMass = 1.0f,.mInertia = JPH::Mat44::sIdentity()};
        // settings.mCollideKinematicVsNonDynamic = true;
        settings.mGravityFactor = 0.0f;
        const auto body = bodyInterface.CreateBody(settings);
        setBodyId(body->GetID());
    }

    void CollisionArea::activate(const float3& position, const quaternion& rotation) {
        createBody(position, rotation);
    }
}
