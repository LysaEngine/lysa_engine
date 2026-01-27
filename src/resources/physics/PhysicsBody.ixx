/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#endif
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.resources.physics_body;

import lysa.math;
import lysa.resources.collision_object;
import lysa.resources.collision_shape;

export namespace lysa {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody : public CollisionObject {
    public:
        void recreateBody();

        ~PhysicsBody() override = default;

    protected:
        void setShape(const std::shared_ptr<CollisionShape> &shape);

        virtual void createBody(const float3& position, const quaternion& rotation, const float3& scale);

#ifdef PHYSIC_ENGINE_JOLT
        JPH::EMotionType motionType;
        JPH::Shape* joltShape{nullptr};
        PhysicsBody(const std::shared_ptr<CollisionShape> &shape,
                    collision_layer layer,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType);
        PhysicsBody(collision_layer layer,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType);
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        physx::PxActorType::Enum actorType;
        PhysicsBody(const std::shared_ptr<Shape>& shape,
                    collision_layer layer,
                    physx::PxActorType::Enum actorType);
        PhysicsBody(collision_layer layer,
                    physx::PxActorType::Enum actorType);
        void createShape(const float3& position, const quaternion& rotation) override;
#endif
    };

}
