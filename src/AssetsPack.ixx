/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.assets_pack;

import vireo;
import lysa.context;
import lysa.exception;
import lysa.log;
import lysa.math;
import lysa.resources.texture;
import lysa.resources.animation;
import lysa.resources.animation_library;
import lysa.resources.image;
import lysa.resources.material;
import lysa.resources.mesh;

export namespace lysa {

    /*
     * Lysa assets pack binary file format containing resources for a scene : meshes, materials, textures and images.<br>
     * It can also be used as a complete scene file since it contains a node tree.<br>
     * This file format is adapted to Lysa and have the following advantages :<br>
     * - Binary file format : fast loading of data without deserialization
     * - Compressed images : images are compressed in GPU-compatible format like the BCn formats to reduce the VRAM usage.<br>
     * - One big images atlas : all images are read and directly uploaded to the GPU in one pass and without using a big CPU buffer.<br>
     * - Pre calculated mip levels : all images mips levels are pre-calculated and compressed.<br>
     * - Pre calculated data : all transforms are pre-calculated.<br>
     * - Pre calculated animations data : all animation data are made relative to the original object.<br>
     *<br>
     *%A Lysa assets pack file can be created from a glTF file using the `gltf2lysa` command line tool.<br>
     *<br>
     * **File format description :**
     * ```
     * Header : global header
     * array<ImageHeader + array<MipLevelInfo, mipLevels>, imagesCount> : images headers
     * array<TextureHeader, texturesCount> : textures headers
     * array<MaterialHeader, materialsCount> : materials headers
     * array<MeshHeader + array<SurfaceInfo, surfacesCount> + array<DataInfo, surfacesCount * uvsCount>, meshesCount> : meshes headers
     * array<NodeHeader + array<uint32, childrenCount>, nodesCount> : nodes headers
     * array<AnimationHeader + array<TrackInfo, tracksCount>, animationCount> : animation headers
     * uint32 : indicesCount
     * array<uint32, indicesCount> : indices data bloc
     * uint32 : positionsCount
     * array<float3, positionsCount> : positions data bloc
     * uint32 : normalsCount
     * array<float3, normalsCount> : normals data bloc
     * uint32 : uvsCount
     * array<vec2, uvsCount> : uvs data bloc
     * uint32 : tangentsCount
     * array<float4, tangentsCount> : tangents data bloc
     * array<float, keysCount> + array<float3, keyCount> for each track, for each animation : animations data
     * array<BCn compressed image, imagesCount> : images data bloc
     *  ```
     */
    class AssetsPack {
    public:
        /*
         * Maximum size of strings in files
         */
        static constexpr auto NAME_SIZE{64};

        /*
         * Magic header thing
         */
        static constexpr char MAGIC[]{ 'A', 'S', 'S', 'E', 'T', 'S' };

        /*
         * Current format version
         */
        static constexpr uint32 VERSION{1};

        /*
         * Global file header
         */
        struct Header {
            //! Magic header thing
            char   magic[6];
            //! Format version
            uint32 version{0};
            //! Total number of images
            uint32 imagesCount{0};
            //! Total number of textures
            uint32 texturesCount{0};
            //! Total number of materials
            uint32 materialsCount{0};
            //! Total number of meshes
            uint32 meshesCount{0};
            //! Total number of scene nodes
            uint32 nodesCount{0};
            uint32 animationsCount{0};
            //! Size in bytes of all the headers
            uint64 headersSize;
        };

        /*
         * Description of an image
         */
        struct ImageHeader {
            //! Image name, copied from the original file name
            char   name[NAME_SIZE];
            //! Image format (vireo::ImageFormat)
            uint32 format;
            //! Width in pixels
            uint32 width;
            //! Height in pixels
            uint32 height;
            //! Number of mips levels, and number of MipLevelInfo elements in the array following this struct
            uint32 mipLevels;
            //! Start of the image, relative to the start of the images data block
            uint64 dataOffset;
            //! Size in bytes of all levels
            uint64 dataSize;
        };

        /*
         * Description of a mip level for an image
         */
        struct MipLevelInfo {
            //! Start of the level, relative to the image dataOffset
            uint64 offset;
            //! Size in bytes of the level
            uint64 size;
        };

        /*
         * Description of a texture
         */
        struct TextureHeader {
            //! Associated image, -1 for a texture without image
            int32  imageIndex{-1};
            //! Minification filter to apply to texture lookups, [VkFilter format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFilter.html)
            uint32 minFilter;
            //! Magnification filter to apply to texture lookups, [VkFilter format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFilter.html)
            uint32 magFilter;
            //! Behavior of sampling with texture coordinates outside an image for U coordinates, [VkSamplerAddressMode format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
            uint32 samplerAddressModeU;
            //! Behavior of sampling with texture coordinates outside an image for V coordinates, [VkSamplerAddressMode format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
            uint32 samplerAddressModeV;
        };

        /*
         * Description of a texture attached to a material
         */
        struct TextureInfo {
            //! Attached texture, -1 if no texture
            int32    textureIndex{-1};
            //! Index of UV coordinates, in the UV DataInfo array of the attached surface
            uint32   uvsIndex;
            //! UV coordinates transform
            float3x3 transform;
        };

        /*
         * Description of a material
         */
        struct MaterialHeader {
            //! Material name
            char        name[NAME_SIZE];
            //! Culling mode CullMode format
            uint32      cullMode;
            //! Transparency, Transparency format
            uint32      transparency;
            //! Alpha scissor for transparency
            float       alphaScissor;
            //! Albedo color
            float4      albedoColor;
            //! Optional albedo texture
            TextureInfo albedoTexture;
            //! Metallic factor
            float       metallicFactor;
            //! Optional metallic texture
            TextureInfo metallicTexture;
            //! Roughness factor
            float       roughnessFactor;
            //! Optional roughness texture
            TextureInfo roughnessTexture;
            //! Emissive factor
            float3      emissiveFactor;
            //! Emissive streng
            float       emissiveStrength;
            //! Optional emissive texture
            TextureInfo emissiveTexture;
            //! Optional normal texture
            TextureInfo normalTexture;
            //! Normal scale
            float       normalScale;
        };

        /*
         * Description of a data bloc (indices, positions, normals, tangents, uv coords)
         */
        struct DataInfo {
            //! First index in the array
            uint32 first;
            //! Number of elements
            uint32 count;
        };

        /*
         * Description of a mesh primitive
         */
        struct SurfaceInfo {
            //! Attached material, -1 if no material
            int32    materialIndex{-1};
            //! Indices array
            DataInfo indices;
            //! Vertices positions array
            DataInfo positions;
            //! Normals array
            DataInfo normals;
            //! Tangents array
            DataInfo tangents;
            //! Number of DataInfo elements in the UV coordinates array of array following this struct
            uint32   uvsCount;
        };

        /*
         * Description of a mesh
         */
        struct MeshHeader {
            //! Name
            char   name[NAME_SIZE];
            //! Number of SurfaceInfo elements in the array following this struct
            uint32 surfacesCount;
        };

        /*
         * Description of a node in the scene
         */
        struct NodeHeader {
            //! Name
            char     name[NAME_SIZE];
            //! Associated mesh, -1 of no mesh. Nodes with mesh will be instantiated as z0::MeshInstance, other nodes as z0::Node
            uint32   meshIndex;
            //! World transform
            float4x4 transform;
            //! Number of children nodes, also the number of elements in the uint32 array following this struct
            uint32   childrenCount;
        };

        struct AnimationHeader {
            char   name[NAME_SIZE];
            uint32 tracksCount;
        };

        struct TrackInfo {
            int32  nodeIndex{-1};
            uint32 type;
            uint32 interpolation;
            uint32 keysCount;
            // + keyCount * float keyTime
            // + keyCount * variant<float3, quat> keyValue
        };

        /*
         * Load a scene from an assets pack file
         */
        template<typename T_OBJECT, typename T_MESH_INSTANCE, typename T_ANIMATION_PLAYER>
        static std::shared_ptr<T_OBJECT> load(const std::string &fileURI, const std::string& rootName = "Root") {
            std::ifstream ifs = ctx().fs.openReadStream(fileURI);
            return load<T_OBJECT, T_MESH_INSTANCE, T_ANIMATION_PLAYER>(ifs, rootName);
        }

        /*
         * Load a scene from an assets pack data stream
         */
        template<typename T_OBJECT, typename T_MESH_INSTANCE, typename T_ANIMATION_PLAYER>
        static std::shared_ptr<T_OBJECT> load(std::ifstream &stream, const std::string& rootName = "Root");

        AssetsPack() = default;

    private:
        static std::vector<std::shared_ptr<vireo::Image>> loadImagesAndTextures(
            const Header& header,
            std::vector<ImageTexture>& textures,
            const vireo::Buffer& stagingBuffer,
            const vireo::CommandList& commandList,
            std::ifstream& stream,
            const std::vector<ImageHeader>&,
            const std::vector<std::vector<MipLevelInfo>>&,
            const std::vector<TextureHeader>&);
    };

    template<typename T_OBJECT, typename T_MESH_INSTANCE, typename T_ANIMATION_PLAYER>
    std::shared_ptr<T_OBJECT> AssetsPack::load(std::ifstream& stream, const std::string& rootName) {
        Header header{};
        auto& imageManager = ctx().res.get<ImageManager>();
        auto& materialManager = ctx().res.get<MaterialManager>();
        auto& meshManager = ctx().res.get<MeshManager>();
        // Read the file global header
        stream.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(header));
        if (header.magic[0] != MAGIC[0] &&
            header.magic[1] != MAGIC[1] &&
            header.magic[2] != MAGIC[2] &&
            header.magic[3] != MAGIC[3] &&
            header.magic[4] != MAGIC[4] &&
            header.magic[5] != MAGIC[5]) {
            throw Exception("Assets pack bad magic");
        }
        if (header.version != VERSION) {
            throw Exception("Assets pack version");
        }
        // print(header);

        // Read the images & mips levels headers
        auto imageHeaders = std::vector<ImageHeader>(header.imagesCount);
        auto levelHeaders = std::vector<std::vector<MipLevelInfo>>(header.imagesCount);
        uint64 totalImageSize{0};
        for (auto imageIndex = 0; imageIndex < header.imagesCount; ++imageIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&imageHeaders[imageIndex]), sizeof(ImageHeader));
            // print(imageHeaders[imageIndex]);
            levelHeaders[imageIndex].resize(imageHeaders[imageIndex].mipLevels);
            stream.read(reinterpret_cast<std::istream::char_type *>(levelHeaders[imageIndex].data()), sizeof(MipLevelInfo) * imageHeaders[imageIndex].mipLevels);
            totalImageSize += imageHeaders[imageIndex].dataSize;
        }

        // Read the textures & materials headers
        auto textureHeaders = std::vector<TextureHeader>(header.texturesCount);
        if (header.texturesCount > 0) {
            stream.read(reinterpret_cast<std::istream::char_type *>(textureHeaders.data()), textureHeaders.size() * sizeof(TextureHeader));
        }
        auto materialHeaders = std::vector<MaterialHeader>(header.materialsCount);
        if (header.materialsCount > 0) {
            stream.read(reinterpret_cast<std::istream::char_type *>(materialHeaders.data()), materialHeaders.size() * sizeof(MaterialHeader));
        }

        // Read the meshes & surfaces headers
        auto meshesHeaders = std::vector<MeshHeader>(header.meshesCount);
        auto surfaceInfo = std::vector<std::vector<SurfaceInfo>> (header.meshesCount);
        auto uvsInfos = std::vector<std::vector<std::vector<DataInfo>>> (header.meshesCount);
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&meshesHeaders[meshIndex]), sizeof(MeshHeader));
            // print(meshesHeaders[meshIndex]);
            surfaceInfo[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            uvsInfos[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            for (auto surfaceIndex = 0; surfaceIndex < meshesHeaders[meshIndex].surfacesCount; ++surfaceIndex) {
                stream.read(reinterpret_cast<std::istream::char_type *>(&surfaceInfo[meshIndex][surfaceIndex]), sizeof(SurfaceInfo));
                // print(surfaceInfo[meshIndex][surfaceIndex]);
                uvsInfos[meshIndex][surfaceIndex].resize(surfaceInfo[meshIndex][surfaceIndex].uvsCount);
                stream.read(reinterpret_cast<std::istream::char_type *>(uvsInfos[meshIndex][surfaceIndex].data()), sizeof(DataInfo) * uvsInfos[meshIndex][surfaceIndex].size());
            }
        }

        // Read the nodes headers
        auto nodeHeaders = std::vector<NodeHeader>(header.nodesCount);
        auto childrenIndexes = std::vector<std::vector<uint32>>(header.nodesCount);
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&nodeHeaders[nodeIndex]), sizeof(NodeHeader));
            childrenIndexes.at(nodeIndex).resize(nodeHeaders[nodeIndex].childrenCount);
            stream.read(reinterpret_cast<std::istream::char_type *>(childrenIndexes.at(nodeIndex).data()), sizeof(uint32) * childrenIndexes[nodeIndex].size());
        }

        auto animationHeaders = std::vector<AnimationHeader>(header.animationsCount);
        auto tracksInfos = std::vector<std::vector<TrackInfo>> (header.animationsCount);
        for (auto animationIndex = 0; animationIndex < header.animationsCount; ++animationIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&animationHeaders.at(animationIndex)), sizeof(AnimationHeader));
            tracksInfos[animationIndex].resize(animationHeaders[animationIndex].tracksCount);
            stream.read(reinterpret_cast<std::istream::char_type *>(tracksInfos[animationIndex].data()), sizeof(TrackInfo) * tracksInfos[animationIndex].size());
        }

        // Read the meshes data
        uint32 count{0};
        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<uint32> indices(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(indices.data()), count * sizeof(uint32));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float3> positions(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(positions.data()), count * sizeof(float3));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float3> normals(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(normals.data()), count * sizeof(float3));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float2> uvs(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(uvs.data()), count * sizeof(float2));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float4> tangents(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(tangents.data()), count * sizeof(float4));

        // INFO(std::format("{} indices, {} positions, {} normals, {} uvs, {} tangents",
            // indices.size(), positions.size(), normals.size(), uvs.size(), tangents.size()));
        // for(auto& pos : positions) {
        //     log(to_string(pos));
        // }

        // Read the animations data
        auto animationLibrary = std::make_shared<AnimationLibrary<T_OBJECT>>();
        for (auto animationIndex = 0; animationIndex < header.animationsCount; animationIndex++) {
            auto anim = std::make_shared<Animation<T_OBJECT>>(
                animationHeaders[animationIndex].tracksCount,
                animationHeaders[animationIndex].name);
            for (auto trackIndex = 0; trackIndex < animationHeaders[animationIndex].tracksCount; trackIndex++) {
                auto& trackInfo = tracksInfos[animationIndex][trackIndex];
                animationLibrary->addAnimation(anim->getName(), anim);
                auto& track = anim->getTrack(trackIndex);
                track.type = static_cast<AnimationType>(trackInfo.type);
                track.interpolation = static_cast<AnimationInterpolation>(trackInfo.interpolation);
                track.keyTime.resize(trackInfo.keysCount);
                stream.read(reinterpret_cast<std::istream::char_type *>(track.keyTime.data()), trackInfo.keysCount * sizeof(float));
                track.duration = track.keyTime.back() + track.keyTime.front();
                track.keyValue.resize(trackInfo.keysCount);
                stream.read(reinterpret_cast<std::istream::char_type *>(track.keyValue.data()), trackInfo.keysCount * sizeof(float3));
            }
        }

        // Read, upload and create the Image and Texture objets (Vireo specific)
        std::vector<ImageTexture> textures;
        textures.reserve(header.imagesCount);
        if (header.imagesCount > 0) {
            auto& asyncQueue = ctx().asyncQueue;
            const auto command = asyncQueue.beginCommand(vireo::CommandType::TRANSFER);
            // Upload all images into VRAM using one big staging buffer
            std::shared_ptr<vireo::Buffer> textureStagingBuffer;
            textureStagingBuffer = asyncQueue.createBuffer(
               command,
               vireo::BufferType::IMAGE_UPLOAD,
               totalImageSize,
               1);
            textureStagingBuffer->map();
            const auto images = loadImagesAndTextures(
                header,
                textures,
                *textureStagingBuffer,
                *command.commandList,
                stream,
                imageHeaders,
                levelHeaders,
                textureHeaders);
             asyncQueue.endCommand(command);
            const auto barriersCommand = asyncQueue.beginCommand(vireo::CommandType::GRAPHIC);
            for (auto textureIndex = 0; textureIndex < header.texturesCount; ++textureIndex) {
                const auto& texture = textureHeaders[textureIndex];
                if (texture.imageIndex != -1) {
                    const auto& imageHeader = imageHeaders[texture.imageIndex];
                    barriersCommand.commandList->barrier(
                        images[textureIndex],
                        vireo::ResourceState::COPY_DST,
                        vireo::ResourceState::SHADER_READ,
                        0,
                        imageHeader.mipLevels);
                }
            }
            asyncQueue.endCommand(barriersCommand);
        }

        // Create the Material objects
        std::unordered_map<pipeline_id, std::vector<unique_id>> pipelineIds;
        std::vector<unique_id> materials(header.materialsCount);
        std::map<unique_id, int> materialsTexCoords;
        for (auto materialIndex = 0; materialIndex < header.materialsCount; ++materialIndex) {
            auto& header = materialHeaders.at(materialIndex);
            auto& material = materialManager.create();
            material.setBypassUpload(true);
            auto textureInfo = [&](const TextureInfo& info) {
                auto texInfo = StandardMaterial::TextureInfo {
                    .transform = info.transform,
                };
                if (info.textureIndex != -1) {
                    texInfo.texture = textures[info.textureIndex];
                }
                materialsTexCoords[material.id] = info.uvsIndex;
                return texInfo;
            };
            material.setCullMode(static_cast<vireo::CullMode>(header.cullMode));
            material.setTransparency(static_cast<Transparency>(header.transparency));
            material.setAlphaScissor(header.alphaScissor);
            material.setAlbedoColor(header.albedoColor);
            material.setDiffuseTexture(textureInfo(header.albedoTexture));
            material.setMetallicFactor(header.metallicFactor);
            material.setMetallicTexture(textureInfo(header.metallicTexture));
            material.setRoughnessFactor(header.roughnessFactor);
            material.setRoughnessTexture(textureInfo(header.roughnessTexture));
            material.setEmissiveFactor(header.emissiveFactor);
            material.setEmissiveTexture(textureInfo(header.emissiveTexture));
            material.setEmissiveStrength(header.emissiveStrength);
            material.setNormalTexture(textureInfo(header.normalTexture));
            material.setNormalScale(header.normalScale > 0.0f ? header.normalScale : 1.0f);
            materials.at(materialIndex) = material.id;
            material.setBypassUpload(false);
            pipelineIds[material.getPipelineId()].push_back(material.id);
        }

        // Create the Mesh, Surface & Vertex objects
        std::vector<unique_id> meshes(header.meshesCount);
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            auto& header   = meshesHeaders[meshIndex];
            auto& mesh     = meshManager.create(std::string(header.name));
            auto& meshVertices = mesh.getVertices();
            auto& meshIndices  = mesh.getIndices();
            // print(header);
            for (auto surfaceIndex = 0; surfaceIndex < header.surfacesCount; ++surfaceIndex) {
                auto &info = surfaceInfo.at(meshIndex)[surfaceIndex];
                // print(info);
                uint32 firstIndex = meshIndices.size();
                uint32 firstVertex  = meshVertices.size();
                auto surface = MeshSurface{firstIndex, info.indices.count};
                // Load indices
                meshIndices.reserve(meshIndices.size() + info.indices.count);
                for(auto i = 0; i < info.indices.count; ++i) {
                    meshIndices.push_back(indices[info.indices.first + i]);
                }
                // Load positions
                meshVertices.resize(meshVertices.size() + info.positions.count);
                for(auto i = 0; i < info.positions.count; ++i) {
                    meshVertices[firstVertex + i] = {
                        .position = positions[info.positions.first + i],
                    };
                }
                // Load normals
                for(auto i = 0; i < info.normals.count; ++i) {
                    meshVertices[firstVertex + i].normal = normals[info.normals.first + i];
                }
                // Load tangents
                for(auto i = 0; i < info.tangents.count; ++i) {
                    meshVertices[firstVertex + i].tangent = tangents[info.tangents.first + i];
                }
                if (info.materialIndex != -1) {
                    // associate material to surface & mesh
                    const auto& material = materials[info.materialIndex];
                    surface.material = material;
                    materialManager.use(material);
                    mesh.getMaterials().insert(material);
                    // load UVs
                    auto texCoord = 0;
                    if (materialsTexCoords.contains(material)) {
                        texCoord = materialsTexCoords.at(material);
                    }
                    if (!uvsInfos.at(meshIndex)[surfaceIndex].empty()) {
                        const auto& texCoordInfo = uvsInfos.at(meshIndex)[surfaceIndex][texCoord];
                        for(auto i = 0; i < texCoordInfo.count; i++) {
                            meshVertices[firstVertex + i].uv = uvs[texCoordInfo.first + i];
                        }
                    }
                } else {
                    // Mesh have no material, use a default one
                    const auto &material =  materialManager.create();
                    surface.material = material.id;
                    materialManager.use(material.id);
                    mesh.getMaterials().insert(material.id);
                }
                // calculate missing tangents
                if (info.tangents.count == 0) {
                    for (auto i = 0; i < meshIndices.size(); i += 3) {
                        auto &vertex1  = meshVertices[meshIndices[i]];
                        auto &vertex2  = meshVertices[meshIndices[i + 1]];
                        auto &vertex3  = meshVertices[meshIndices[i + 2]];
                        float3 edge1    = vertex2.position - vertex1.position;
                        float3 edge2    = vertex3.position - vertex1.position;
                        float2 deltaUV1 = vertex2.uv - vertex1.uv;
                        float2 deltaUV2 = vertex3.uv - vertex1.uv;

                        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                        float3  tangent{
                            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
                        };
                        vertex1.tangent = float4{tangent, 1.0};
                        vertex2.tangent = float4{tangent, 1.0};
                        vertex3.tangent = float4{tangent, 1.0};
                    }
                }
                mesh.getSurfaces().push_back(surface);
            }
            mesh.buildAABB();
            meshes[meshIndex] = mesh.id;
        }
        materialManager.flush();
        meshManager.flush();

        std::vector<std::shared_ptr<T_OBJECT>> nodes(nodeHeaders.size());
        for (auto nodeIndex = 0; nodeIndex < nodeHeaders.size(); ++nodeIndex) {
            if (nodeHeaders[nodeIndex].meshIndex != -1) {
                const auto mi = std::make_shared<T_MESH_INSTANCE>(
                    meshManager[meshes[nodeHeaders[nodeIndex].meshIndex]],
                    nodeHeaders[nodeIndex].name);
                mi->setCastShadows(true);
                nodes[nodeIndex] = mi;
            }
            else {
                nodes[nodeIndex] =  std::make_shared<T_OBJECT>(nodeHeaders[nodeIndex].name);
            }
            nodes[nodeIndex]->setTransform(nodeHeaders[nodeIndex].transform);
        }

        // Build the scene tree
        for (auto nodeIndex = 0; nodeIndex < nodeHeaders.size(); ++nodeIndex) {
           auto* parent = nodes[nodeIndex].get();
           for (auto i = 0; i < nodeHeaders[nodeIndex].childrenCount; i++) {
               const auto& child = nodes[childrenIndexes[nodeIndex][i]];
               parent->addChild(child);
           }
        }

        // find the top nodes, with no parents
        auto root = std::make_shared<T_OBJECT>(rootName);
        for (const auto& instance : nodes) {
            if (!instance->haveParent()) {
                root->addChild(instance);
            }
        }

        if (!animationHeaders.empty()) {
            auto animationPlayer = std::make_shared<T_ANIMATION_PLAYER>();
            animationPlayer->add("", animationLibrary);
            for (auto animationIndex = 0; animationIndex < animationHeaders.size(); animationIndex++) {
                auto animation = animationLibrary->getAnimation(animationHeaders[animationIndex].name);
                for (auto trackIndex = 0; trackIndex < animationHeaders[animationIndex].tracksCount; trackIndex++) {
                    const auto nodeIndex = tracksInfos[animationIndex][trackIndex].nodeIndex;
                    animation->getTrack(trackIndex).target = nodes[nodeIndex].get();
                    Log::info(nodes[nodeIndex]->getRelativePath(root));
                    animation->getTrack(trackIndex).path = nodes[nodeIndex]->getRelativePath(root);
                }
            }
            root->addChild(animationPlayer);
        }

        for (auto& texture : textures) {
            auto& image = imageManager[texture.image];
            if (image.refCounter == 0) {
                Log::warning("Image ", image.getName(), " not used in the assets pack");
                imageManager.destroy(texture.image);
            }
        }

        // Update renderers pipelines in current rendering targets
        //ctx().res.get<RenderTargetManager>().updatePipelines(pipelineIds);  XXX

        return root;
    }


}
