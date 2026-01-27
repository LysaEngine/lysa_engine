/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.physics.engine;

import std;
import lysa.math;
import lysa.physics.configuration;
import lysa.physics.physics_material;
import lysa.renderers.configuration;
import lysa.renderers.debug_renderer;

export namespace lysa {

    /**
     * Lists the available physics backends supported by the engine.
     *  - JOLT: Uses the Jolt Physics library.
     *  - PHYSX: Uses NVIDIA PhysX.
     */
    enum class PhysicsEngineType {
        JOLT,
        PHYSX
    };

    /**
     * Physics world interface.
     *
     * A PhysicsWorld encapsulates the simulation world (bodies, constraints,
     * queries) and advances it with a fixed time step. Concrete implementations
     * are provided by each backend (Jolt / PhysX).
     */
    class PhysicsWorld {
    public:
        /**
         * Steps the physics simulation by the given delta time (seconds).
         * Implementations may internally clamp or subdivide the step.
         */
        virtual void update(float deltaTime) = 0;

        /**
         * Emits debug visualization primitives to the provided DebugRenderer.
         * Used to display bodies, shapes, contacts, etc. when enabled.
         */
        virtual void debug(DebugRenderer& debugRenderer) = 0;

        /**
         * Returns the current gravity vector applied to dynamic bodies.
         */
        virtual float3 getGravity() const = 0;

        virtual ~PhysicsWorld() = default;
    };

    /**
     * Abstract factory/entry point for physics backends.
     */
    class PhysicsEngine {
    public:
        /**
         * Creates a physics engine using the active backend.
         */
        static std::unique_ptr<PhysicsEngine> create(const PhysicsEngineConfiguration& config);

        /**
         * Creates a new physics world with optional debug settings.
         */
        virtual std::unique_ptr<PhysicsWorld> createScene(const DebugConfiguration& debugConfig)  = 0;

        /**
         * Creates a new physics material with the specified properties.
         * @param friction    Coefficient of friction in [0..+inf).
         * @param restitution Bounciness coefficient in [0..1].
         */
        virtual PhysicsMaterial* createMaterial(
            float friction = 0.5f,
            float restitution = 0.0f) const = 0;

        /**
         * Sets the combine mode used when resolving restitution between two materials.
         */
        virtual void setRestitutionCombineMode(PhysicsMaterial* physicsMaterial, CombineMode combineMode) const = 0;

        /**
         * Duplicates a material instance (backend‑specific).
         */
        virtual PhysicsMaterial* duplicateMaterial(const PhysicsMaterial* orig) const = 0;

        /**
         * Returns the physics backend compiled in use (JOLT or PHYSX).
         */
        static PhysicsEngineType getEngineType();

        virtual ~PhysicsEngine() = default;
    };

}