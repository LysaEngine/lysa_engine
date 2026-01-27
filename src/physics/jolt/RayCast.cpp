/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
module lysa.resources.ray_cast;

import lysa;
import lysa.physics.jolt.engine;

namespace lysa {

    RayCast::RayCast(const float3 &target, const collision_layer layer) :
        target(target) {
        setCollisionLayer(layer);
    }

    RayCast::~RayCast() {
        ctx().events.unsubscribe(onPhysicsProcessHandler);
    }

    void RayCast::update() {
        const auto& position = getGlobalPosition();
        const auto worldDirection = localPositionToGlobalPosition(target) - position;
        const JPH::RRayCast ray{
            JPH::Vec3{position.x, position.y, position.z },
            JPH::Vec3{worldDirection.x, worldDirection.y, worldDirection.z}
        };
        JPH::RayCastResult result;
        auto& physicsScene = static_cast<JoltPhysicsWorld&>(*ctx().physicsWorld.get());
        if (physicsScene.getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
                ray,
                result,
                broadPhaseLayerFilter,
                *objectLayerFilter,
                *this)) {
            collider = reinterpret_cast<CollisionObject *>(physicsScene.getBodyInterface().GetUserData(result.mBodyID));
            const auto posInRay = ray.GetPointOnRay(result.mFraction);
            hitPoint = float3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        } else {
            collider = nullptr;
        }
    }

    void RayCast::setCollisionLayer(const collision_layer layer) {
        auto& engine = static_cast<JoltPhysicsEngine&>(*ctx().physicsEngine);
        collisionLayer = layer;
        objectLayerFilter = std::make_unique<JPH::DefaultObjectLayerFilter>(
            engine.getObjectLayerPairFilter(),
            collisionLayer);
        if (onPhysicsProcessHandler == INVALID_ID) {
            onPhysicsProcessHandler = ctx().events.subscribe(MainLoopEvent::PHYSICS_PROCESS, [this](const Event&) {
               if (isProcessed()) {
                   update();
               }
           });
        }
    }

    bool RayCast::ShouldCollideLocked(const JPH::Body &inBody) const {
        const auto *co = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return (co != nullptr) && (!(excludeParent && (isParent(co)))) && isProcessed() && co->isProcessed();
    }

}
