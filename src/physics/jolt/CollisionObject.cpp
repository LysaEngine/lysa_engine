/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
module lysa.resources.collision_object;

import lysa.context;
import lysa.log;
import lysa.physics.jolt.engine;

namespace lysa {

    CollisionObject::~CollisionObject() {
        releaseResources();
    }

    CollisionObject::CollisionObject(
        const std::shared_ptr<CollisionShape>& shape,
        const uint32 layer):
        collisionLayer{layer},
        shape{shape},
        bodyInterface(static_cast<JoltPhysicsWorld*>(ctx().physicsWorld.get())->getBodyInterface()),
        physicsSystem(static_cast<JoltPhysicsWorld*>(ctx().physicsWorld.get())->getPhysicsSystem()) {
    }

    CollisionObject::CollisionObject(
        const uint32 layer):
        collisionLayer{layer},
        bodyInterface(static_cast<JoltPhysicsWorld*>(ctx().physicsWorld.get())->getBodyInterface()),
        physicsSystem(static_cast<JoltPhysicsWorld*>(ctx().physicsWorld.get())->getPhysicsSystem()) {
    }

    CollisionObject::CollisionObject(const CollisionObject& other):
        CollisionObject(other.shape, other.collisionLayer){
    }

    void CollisionObject::setBodyId(const JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()), getName());
    }

    void CollisionObject::releaseResources() {
        if (!bodyId.IsInvalid()) {
            if (bodyInterface.IsAdded(bodyId)) {
                bodyInterface.RemoveBody(bodyId);
            }
            bodyInterface.DestroyBody(bodyId);
            bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
        }
    }

    void CollisionObject::setCollisionLayer(const collision_layer layer) {
        collisionLayer = layer;
        if (!bodyId.IsInvalid()) {
            bodyInterface.SetObjectLayer(bodyId, collisionLayer);
        }
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        if (bodyId.IsInvalid()) { return false; }
        return physicsSystem.WereBodiesInContact(bodyId, obj->bodyId);
    }

    void CollisionObject::setPositionAndRotation(const float3& position, const quaternion& rotation) {
        if (updating || bodyId.IsInvalid()|| !bodyInterface.IsAdded(bodyId)) {
            return;
        }
        const auto& quat = normalize(rotation);
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::scale(const float scale) {
        if (bodyId.IsInvalid()|| !bodyInterface.IsAdded(bodyId)) {
            return;
        }
        if (scale != 1.0f) {
            bodyInterface.SetShape(
                bodyId,
                new JPH::ScaledShape(
                    bodyInterface.GetShape(bodyId),
                    JPH::Vec3{scale, scale, scale}),
                false,
                activationMode);
        }
    }

    void CollisionObject::update(float3& position, quaternion& rotation) {
        if (bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) { return; }
        updating = true;
        JPH::Vec3 pos;
        JPH::Quat rot;
        bodyInterface.GetPositionAndRotation(bodyId, pos, rot);
        position = float3{pos.GetX(), pos.GetY(), pos.GetZ()};
        rotation = quaternion{rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW()};
        updating = false;
    }

    void CollisionObject::activate(const float3& position, const quaternion& rotation) {
        bodyInterface.SetObjectLayer(bodyId, collisionLayer);
        if (isProcessed() && isVisible()) {
            bodyInterface.AddBody(bodyId, activationMode);
            setPositionAndRotation(position, rotation);
        }
    }

    void CollisionObject::deactivate() const {
        if (!bodyId.IsInvalid()) {
            if (bodyInterface.IsAdded(bodyId)) {
                bodyInterface.RemoveBody(bodyId);
            }
        }
    }

    void CollisionObject::pause() const {
        if (!bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.RemoveBody(bodyId);
        }
    }

    void CollisionObject::resume(const float3& position, const quaternion& rotation) {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (isVisible()) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.AddBody(bodyId, activationMode);
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer);
                setPositionAndRotation(position, rotation);
            }
        }
    }

    void CollisionObject::show(const float3& position, const quaternion& rotation) {
        if (!bodyId.IsInvalid() && !this->isVisible()) {
            if (!bodyInterface.IsAdded(bodyId)) {
                bodyInterface.AddBody(bodyId, activationMode);
            }
            bodyInterface.SetObjectLayer(bodyId, collisionLayer);
            setPositionAndRotation(position, rotation);
        }
    }

    void CollisionObject::hide() const {
        if (!bodyId.IsInvalid() && this->isVisible()) {
            if (bodyInterface.IsAdded(bodyId)) {
                bodyInterface.RemoveBody(bodyId);
            }
        }
    }



}
