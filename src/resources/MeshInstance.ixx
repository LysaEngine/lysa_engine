/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.mesh_instance;

import vireo;
import lysa.aabb;
import lysa.math;
import lysa.resources;
import lysa.resources.manager;
import lysa.resources.material;
import lysa.resources.mesh;

export namespace lysa {

    /**
     * Mesh instance data in GPU memory
     */
    struct MeshInstanceData {
        /** World transformation matrix */
        float4x4 transform;
        /** Minimum point of the AABB */
        float3   aabbMin;
        /** Maximum point of the AABB */
        float3   aabbMax;
        /** Visibility flag (1 if visible, 0 otherwise) */
        uint     visible;
        /** Shadow casting flag (1 if casting shadows, 0 otherwise) */
        uint     castShadows;
    };

    /**
    * Instance that holds a Mesh.
    */
    class MeshInstance : public UniqueResource {
    public:
        MeshInstance(const MeshInstance& orig);

        /**
         * Constructor
         * @param mesh Reference to the mesh
         * @param name Optional name for the instance
         */
        MeshInstance(const Mesh& mesh, const std::string& name = "");

        MeshInstance(const MeshInstance& mi, const std::string& name);

        /**
         * Returns the mesh associated with the instance
         */
        const Mesh& getMesh() const { return mesh; }

        /**
         * Returns `true` if the instance is visible
         */
        bool isVisible() const { return visible; }

        /**
         * Sets the visibility of the instance
         */
        void setVisible(const bool visible) { this->visible = visible; }

        /**
         * Returns `true` if the instance casts shadows
         */
        bool isCastShadows() const { return castShadows; }

        /**
         * Sets whether the instance casts shadows
         */
        void setCastShadows(const bool castShadows) { this->castShadows = castShadows; }

        /**
         * Returns the world-space AABB of the instance
         */
        const AABB& getAABB() const { return worldAABB; }

        /**
         * Sets the world-space AABB of the instance
         */
        void setAABB(const AABB& aabb) { worldAABB = aabb; }

        /**
         * Returns the world transformation matrix of the instance
         */
        const float4x4& getTransform() const { return worldTransform; }

        /**
         * Sets the world transformation matrix of the instance
         */
        void setTransform(const float4x4& transform) { worldTransform = transform; }

        /**
         * Returns the material ID used by a specific surface
         */
        unique_id getSurfaceMaterial(uint32 surfaceIndex) const;

        /**
         * Sets an override material ID for a specific surface
         */
        void setSurfaceOverrideMaterial(const uint32 surfaceIndex, const unique_id materialId) {
            materialsOverride[surfaceIndex] = materialId;
        }

        /**
         * Removes the override material ID for a specific surface
         */
        void removeSurfaceOverrideMaterial(const uint32 surfaceIndex) {
            materialsOverride.erase(surfaceIndex);
        }

        /**
         * Returns the override material ID for a specific surface
         */
        unique_id getSurfaceOverrideMaterial(const uint32 surfaceIndex) const;

        /**
         * Returns the mesh instance data for GPU consumption
         */
        MeshInstanceData getData() const;

        /** Destructor */
        ~MeshInstance() override;

    private:
        /* Reference to the material manager */
        MaterialManager& materialManager;
        /* Reference to the mesh manager */
        MeshManager& meshManager;
        /* Reference to the mesh */
        const Mesh& mesh;
        /* Name of the instance */
        const std::string name;
        /* Visibility flag */
        bool visible{true};
        /* Shadow casting flag */
        bool castShadows{false};
        /* World-space AABB */
        AABB worldAABB{};
        /* World transformation matrix */
        float4x4 worldTransform{float4x4::identity()};
        /* Map of surface indices to override material IDs */
        std::unordered_map<uint32, unique_id> materialsOverride;
    };

}

