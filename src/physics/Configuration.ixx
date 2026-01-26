/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.physics.configuration;

import std;
import lysa.math;

export namespace lysa {

    /**
     * Describes which layers can collide with a given layer.
     * Used to build a collision filtering table at engine initialization.
     */
    struct LayerCollideWith {
        /** The source collision layer. */
        collision_layer layer;
        /** List of layers that are allowed to collide with 'layer'. */
        std::vector<uint32> collideWith;
    };

    /**
     * Flattened description of layer‑to‑layer collision rules.
     */
    struct LayerCollisionTable {
        /** Total number of layers defined in the project. */
        uint32 layersCount;
        /** Per‑layer entries describing which other layers they collide with. */
        std::vector<LayerCollideWith> layersCollideWith;
    };

    /**
     * High‑level physics engine configuration.
     */
    struct PhysicsConfiguration {
        /** Layers vs Layers collision table. */
        LayerCollisionTable layerCollisionTable;
    };

    //! Coloring scheme of collision shapes (only supported by Jolt)
    enum class PhysicsDebugShapeColor {
        //! Random color per instance
        InstanceColor,
        //! Convex = green, scaled = yellow, compound = orange, mesh = red
        ShapeTypeColor,
        //! Static = grey, keyframed = green, dynamic = random color per instance
        MotionTypeColor,
        //! Static = grey, keyframed = green, dynamic = yellow, sleeping = red
        SleepColor,
    };

    /**
     * Configuration of the in-game debug
     */
    struct PhysicsDebugConfiguration {
        //! Enable the debug visualization
        bool enabled{false};
        //! If the debug renderer is enabled, display the debug at startup
        bool displayAtStartup{true};
        //! Draw with depth-testing
        bool depthTestEnable{true};
        //! Draw coordinate system (x = red, y = green, z = blue)
        bool drawCoordinateSystem{false};
        //! Coordinate system draw scale
        float coordinateSystemScale{1.0f};
        //! Draw all the rays of the RayCast objects
        bool drawRayCast{false};
        //! Color for the non-colliding rays
        float4 rayCastColor{0.0f, 0.5f, 1.0f, 1.0f};
        //! Color for the colliding rays
        float4 rayCastCollidingColor{0.95f, 0.275f, 0.76f, 1.0f};
        //! Draw the collision shapes of all collision objects
        bool drawShape{true};
        //! Coloring scheme to use for collision shapes
        PhysicsDebugShapeColor shapeColor{PhysicsDebugShapeColor::ShapeTypeColor};
        //! Draw a bounding box per collision object
        bool drawBoundingBox{false};
        //! Draw the velocity vectors
        bool drawVelocity{false};
        //! Draw the center of mass for each collision object
        bool drawCenterOfMass{false};
    };
}
