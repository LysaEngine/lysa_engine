/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.collision_shape;

import lysa.context;

namespace lysa {

    CollisionShape::CollisionShape(const PhysicsMaterial* material):
        material(material ?
            ctx().physicsEngine->duplicateMaterial((material)):
            ctx().physicsEngine->createMaterial()) {
    }

}
