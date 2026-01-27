/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
module lysa.resources.collision_shape;

import lysa.exception;
import lysa.physics.physics_engine;

namespace lysa {

    AABBCollisionShape::AABBCollisionShape(
        const AABB& aabb,
        const PhysicsMaterial* material):
        CollisionShape{material} {
        const auto& extends = (aabb.max - aabb.min) * 0.5f;
        shapeSettings = new JPH::BoxShapeSettings(
            JPH::Vec3(extends.x, extends.y, extends.z),
            JPH::cDefaultConvexRadius,
            reinterpret_cast<const JPH::PhysicsMaterial*>(this->material));
    }

    BoxCollisionShape::BoxCollisionShape(
        const float3& extends,
        const PhysicsMaterial* material):
        CollisionShape{material}, extends
        {extends} {
        if (extends.x <= 0.2 || extends.y <= 0.2 || extends.z <= 0.2) {
            throw Exception("Invalid extends for BoxShape : extends must be greater than 0.2");
        }
        shapeSettings = new JPH::BoxShapeSettings(
            JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2),
            JPH::cDefaultConvexRadius,
            reinterpret_cast<const JPH::PhysicsMaterial*>(this->material));
    }

    // BoxCollisionShape& BoxCollisionShape::duplicate() const {
    //     auto dup = std::make_shared<BoxShape>(extends, material, getName());
    //     dup->shapeSettings = new JPH::BoxShapeSettings(
    //         JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2),
    //         JPH::cDefaultConvexRadius,
    //         reinterpret_cast<JPH::PhysicsMaterial*>(this->material));
    //     return dup;
    // }

    SphereCollisionShape::SphereCollisionShape(
        const float radius,
        const PhysicsMaterial* material):
        CollisionShape{material},
        radius{radius} {
        shapeSettings = new JPH::SphereShapeSettings(
            radius,
            reinterpret_cast<const JPH::PhysicsMaterial*>(this->material));
    }

    JPH::ShapeSettings* MeshCollisionShape::getShapeSettings() {
        const auto &vertices = mesh.getVertices();
        JPH::VertexList vertexList;
        vertexList.reserve(vertices.size());
        for (const auto &vertex : vertices) {
            auto point = mul(float4{vertex.position, 1.0f}, transform);
            vertexList.push_back(JPH::Float3{point.x, point.y, point.z});
        }
        const auto & indices =mesh.getIndices();
        JPH::IndexedTriangleList triangles;
        JPH::PhysicsMaterialList materials;
        const auto joltMaterial = reinterpret_cast<const JPH::PhysicsMaterial*>(material);
        triangles.reserve(indices.size()/3);
        for (int i = 0; i < indices.size(); i += 3) {
            triangles.push_back({indices[i + 0], indices[i + 1], indices[i + 2]});
            materials.push_back(joltMaterial);
        }
        shapeSettings = new JPH::MeshShapeSettings(vertexList, triangles);
        return shapeSettings;
    }

    JPH::ShapeSettings* ConvexHullCollisionShape::getShapeSettings() {
        std::list<float3> points;
        for (const auto &vertex : mesh.getVertices()) {
            auto point = mul(float4{vertex.position, 1.0f}, transform);
            points.push_back(point.xyz);
        }
        JPH::Array<JPH::Vec3> jphPoints;
        for (const auto &vertex : points) {
            jphPoints.push_back(JPH::Vec3{vertex.x, vertex.y, vertex.z});
        }
        shapeSettings = new JPH::ConvexHullShapeSettings(
            jphPoints,
            JPH::cDefaultConvexRadius,
            reinterpret_cast<const JPH::PhysicsMaterial*>(material));
        return shapeSettings;
    }

    StaticCompoundCollisionShape::StaticCompoundCollisionShape(std::vector<CollisionSubShape> &subshapes) :
        CollisionShape{nullptr} {
        const auto settings = new JPH::StaticCompoundShapeSettings();
        for (auto &subshape : subshapes) {
            const auto quat = quaternion{subshape.rotation};
            settings->AddShape(JPH::Vec3{subshape.position.x, subshape.position.y, subshape.position.z},
                               JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                               subshape.shape.getShapeSettings());
        }
        shapeSettings = settings;
    }

}
