/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.kinematic_body;

import std;
import lysa.types;
import lysa.resources.physics_body;
import lysa.resources.collision_shape;

export namespace lysa {

    /**
     * Physics body moved by velocities only, does not respond to forces.
     */
    class KinematicBody : public PhysicsBody {
    public:
        /**
         * Creates a KinematicBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit KinematicBody(const std::shared_ptr<CollisionShape>& shape,
                               collision_layer layer = 0);

        /**
         * Creates a KinematicBody without a collision shape,
         */
        explicit KinematicBody();

        ~KinematicBody() override = default;
    };

}
