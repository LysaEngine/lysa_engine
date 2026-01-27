/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.static_body;

import std;
import lysa.types;
import lysa.resources.physics_body;
import lysa.resources.collision_shape;

export namespace lysa {

    /**
     * %A 3D physics body that can't be moved by external forces or velocity.
     */
    class StaticBody : public PhysicsBody {
    public:
        /**
         * Creates a StaticBody with a given collision `shape`, 
         * belonging to the `layer` layers.
         */
        StaticBody(const std::shared_ptr<CollisionShape>& shape,
                   collision_layer layer);

        /**
         * Creates a StaticBody without a collision shape`
         * belonging to the `layer` layers
         */
        StaticBody(collision_layer layer);

        /**
        * Creates a StaticBody without a collision shape
        */
        StaticBody();

        ~StaticBody() override = default;
    };

}
