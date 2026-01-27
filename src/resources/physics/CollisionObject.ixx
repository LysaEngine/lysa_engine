/*
 * Copyright (c) 2024-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#endif
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.resources.collision_object;

import lysa.math;
import lysa.resources;
import lysa.resources.collision_shape;
#ifdef PHYSIC_ENGINE_JOLT
import lysa.physics.jolt.engine;
#endif
#ifdef PHYSIC_ENGINE_PHYSX
import lysa.physics.physx.engine;
#endif

export namespace lysa {

    struct CollisionObjectEvent {
        /**
        * Signal called whenever a new contact point is detected and reports the first contact point in a CollisionObject::Collision<br>
        * For characters, called whenever the character collides with a body.
        */
        static constexpr auto START{"COLLISION_STARTS"};
        /**
        * Signal called whenever a contact is detected that was also detected last update and reports the first contact point in a CollisionObject::Collision<br>
        * Never called for characters since on_collision_added is called during the whole contact.
        */
        static constexpr auto PERSISTS{"COLLISION_PERSISTS"};
    };

    /**
     * Base class for 3D physics objects.
     */
    class CollisionObject : public UnmanagedResource {
    public:
        CollisionObject(const CollisionObject&);

        ~CollisionObject() override {
            releaseResources();
        }

        /**
         * Collision data for the CollisionObject::on_collision_starts and CollisionObject::on_collision_persists signal
         */
        struct Collision {
            /** World space position of the first contact point on the colliding `object` or, if the collision is detected by Character::getCollisions(), on the character */
            float3 position;
            /** Normal for this collision, the direction along which to move the `object` out of collision along the shortest path.  */
            float3 normal;
            /** Colliding object */
            CollisionObject *object;
        };

        /**
         * The physics layers this CollisionObject is in.
         */
        auto getCollisionLayer() const { return collisionLayer; }

        /**
         * Sets the collision layer
         */
        virtual void setCollisionLayer(uint32 layer);

        /**
         * Returns `true` if `obj` were in contact with the object during the last simulation step
         */
        bool wereInContact(const CollisionObject *obj) const;

        void scale(float scale) const;

        virtual void activate(const float3& position, const quaternion& rotation);

        void deactivate() const;

        void pause() const;

        void resume(const float3& position, const quaternion& rotation);

        void show(const float3& position, const quaternion& rotation);

        void hide() const;

        virtual void setPositionAndRotation(const float3& position, const quaternion& rotation);

        virtual bool isProcessed() const { return true; }

        virtual bool isCharacter() const { return false; }

        virtual bool isVisible() const { return true; }

    protected:
        bool updating{false};
        collision_layer collisionLayer;
        std::shared_ptr<CollisionShape> shape{nullptr};

        CollisionObject(const std::shared_ptr<CollisionShape>&shape, collision_layer layer);

        CollisionObject(collision_layer layer);

        void releaseResources();

#ifdef PHYSIC_ENGINE_JOLT
        JPH::BodyInterface& bodyInterface;
        JPH::PhysicsSystem& physicsSystem;
        JPH::BodyID bodyId{JPH::BodyID::cInvalidBodyID};
        JPH::EActivation activationMode{JPH::EActivation::Activate};

        void setBodyId(JPH::BodyID id);
        auto getBodyId() const { return bodyId; }
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        PhysXPhysicsEngine& physX;
        physx::PxRigidActor *actor{nullptr};
        physx::PxScene* scene{nullptr};
        std::list<physx::PxShape*> shapes;

        void setActor(physx::PxRigidActor *actor);

        virtual void createShape() {}

        physx::PxScene* getPxScene() const;

    public:
        auto getActor() const { return actor; }
#endif
    };

}
