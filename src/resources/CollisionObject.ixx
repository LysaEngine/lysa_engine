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

        ~CollisionObject() override;

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

        virtual bool isProcessed() const = 0;

        virtual bool isCharacter() const = 0;

    protected:
        bool updating{false};
        collision_layer collisionLayer;
        std::shared_ptr<CollisionShape> shape{nullptr};

        CollisionObject(const std::shared_ptr<CollisionShape>&shape,
                        collision_layer layer);

        CollisionObject(collision_layer layer);

        virtual void setPositionAndRotation(const float3& position, const quaternion& rotation);

        void releaseResources();

#ifdef PHYSIC_ENGINE_JOLT
        JPH::BodyInterface* bodyInterface{nullptr};
        JPH::EActivation activationMode{JPH::EActivation::Activate};

        auto getBodyId() const { return bodyId; }
        void setBodyId(JPH::BodyID id);

        friend class Character;
        JPH::BodyID bodyId{JPH::BodyID::cInvalidBodyID};
        JPH::BodyInterface* getBodyInterface() const;
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
