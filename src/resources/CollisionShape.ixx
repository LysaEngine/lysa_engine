/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#endif
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.resources.collision_shape;

import std;
import lysa.aabb;
import lysa.math;
import lysa.physics.physics_material;
import lysa.resources;
#ifdef PHYSIC_ENGINE_PHYSX
import lysa.application;
import lysa.physics.physx.engine;
#endif

export namespace lysa {

    /**
     * Base class for all collision shapes
     */
    class CollisionShape : public UnmanagedResource {
    public:
        CollisionShape(const PhysicsMaterial* material, const std::string &resName);

        auto& getMaterial() const { return *material; }

    protected:
        PhysicsMaterial* material;

#ifdef PHYSIC_ENGINE_JOLT
    public:
        virtual JPH::ShapeSettings* getShapeSettings() { return shapeSettings; }
    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};
#endif
#ifdef PHYSIC_ENGINE_PHYSX
    public:
        virtual std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const { return nullptr; }
    protected:
        // std::unique_ptr<physx::PxGeometry> geometry;
        static auto getPhysx() {
            return dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine()).getPhysics();
        }
#endif
    };

    /**
     * Box-shaped collision Shape
     */
    class BoxCollisionShape : public CollisionShape {
    public:
        /**
         * Creates a BoxShape with the given extents
         */
        BoxCollisionShape(
            const float3& extends,
            PhysicsMaterial* material = nullptr,
            const std::string &resName = "BoxShape");

#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        const float3 extends;
    };

    /**
     * Sphere-shaped collision Shape
     */
    class SphereCollisionShape : public CollisionShape {
    public:
        /**
         * Creates a SphereShape with the given radius
         */
        SphereCollisionShape(
            float radius,
            const PhysicsMaterial* material = nullptr,
            const std::string &resName = "SphereShape");

#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        const float radius;
        // SphereShape(const std::string &resName, PhysicsMaterial* material) : Shape(material, resName), radius {0}{}
    };

    /**
     * AABB-based collision %Shape
     */
    class AABBCollisionShape : public CollisionShape {
    public:
        /**
         * Creates an AABBShape for a given node
         */
        AABBCollisionShape(
            const AABB& aabb,
            const PhysicsMaterial* material = nullptr);

#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        float3 extends;
        AABBCollisionShape(
            PhysicsMaterial* material,
            const std::string &resName) :
            CollisionShape{material, resName} {}
    };

}
