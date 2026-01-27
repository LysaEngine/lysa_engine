/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#endif
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.resources.ray_cast;

import lysa.math;
import lysa.resources;
import lysa.resources.collision_object;

export namespace lysa {

    /**
     * %A ray in 3D space, used to find the first CollisionObject it intersects.
     */
    class RayCast : public UnmanagedResource, public
#ifdef PHYSIC_ENGINE_JOLT
        JPH::BodyFilter
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        physx::PxQueryFilterCallback
#endif
    {
    public:
        /**
         * Creates a RayCast
         * @param target The ray's destination point, relative to the RayCast's position
         * @param layer The ray's collision layer
         */
        RayCast(const float3& target, collision_layer layer);

        /**
         * Creates an inactive RayCast
         */
        RayCast() = default;

        ~RayCast() override;

        /**
         * Returns whether any object is intersecting with the ray's vector (considering the vector length).
         */
        auto isColliding() const { return collider != nullptr; }

        /**
         * Returns the first object that the ray intersects, or `nullptr` if no object is intersecting the ray
         */
        auto getCollider() const { return collider; }

        /**
         * Returns the collision point at which the ray intersects the closest object, in the global coordinate system
         */
        auto getCollisionPoint() const { return hitPoint; }

        /**
         * If `true`, collisions will be ignored for this RayCast's immediate parent.
         */
        void setExcludeParent(const bool exclude) { excludeParent = exclude; }

        /**
         * Updates the collision information for the ray immediately,
         * without waiting for the next physics update
         */
        void update();

        /**
         * Sets the ray's destination point, relative to the RayCast's position.
         */
        void setTarget(const float3& target) {  this->target = target; };

        /**
         * Returns the ray target
         */
        const auto& getTarget() const { return this->target; }

        void setCollisionLayer(collision_layer layer);

        virtual float3 getGlobalPosition() const = 0;

        virtual float3 localPositionToGlobalPosition(const float3& position) const = 0;

        virtual bool isProcessed() const = 0;

        virtual bool isParent(const CollisionObject* collisionObject) const = 0;

    protected:
        bool active{false};
        float3 target{};
        float3 hitPoint{};
        collision_layer collisionLayer{};
        bool excludeParent{true};
        CollisionObject* collider{nullptr};
        unique_id onPhysicsProcessHandler{INVALID_ID};

#ifdef PHYSIC_ENGINE_JOLT
        JPH::BroadPhaseLayerFilter broadPhaseLayerFilter{};
        std::unique_ptr<JPH::ObjectLayerFilter> objectLayerFilter;
        bool ShouldCollideLocked(const JPH::Body &inBody) const override;
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        physx::PxQueryHitType::Enum preFilter(
            const physx::PxFilterData& filterData,
            const physx::PxShape* shape,
            const physx::PxRigidActor* actor,
            physx::PxHitFlags& queryFlags) override;
        physx::PxQueryHitType::Enum postFilter(
            const physx::PxFilterData& filterData,
            const physx::PxQueryHit& hit,
            const physx::PxShape* shape,
            const physx::PxRigidActor* actor) override;
#endif
    };

}
