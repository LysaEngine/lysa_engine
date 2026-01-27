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
export module lysa.resources.collision_area;

import lysa.math;
import lysa.resources.collision_object;
import lysa.resources.collision_shape;

export namespace lysa {

    /**
     * Collision sensor that reports contacts with other bodies.
     */
    class CollisionArea : public CollisionObject {
    public:
        /**
         * Creates a CollisionArea using the given geometric `shape`
         * to detect collision with bodies having a layer in the `mask` value.
         * @param shape The collision shape
         * @param layer The collision layer
         */
        CollisionArea(const std::shared_ptr<CollisionShape>& shape,  const collision_layer layer) :
            CollisionObject(shape, layer) {
            setShape(shape);
        }

        /**
         * Creates a CollisionArea without a collision shape
         */
        CollisionArea() : CollisionObject(0) {}

        ~CollisionArea() override = default;

        void activate(const float3& position, const quaternion& rotation) override;

        /**
         * Sets the collision shape of the area
         */
        void setShape(const std::shared_ptr<CollisionShape> &shape);

    protected:
        virtual void createBody(const float3& position, const quaternion& rotation);

#ifdef PHYSIC_ENGINE_JOLT
        JPH::Shape* joltShape{nullptr};
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        void createShape() override;
#endif
    };


}
