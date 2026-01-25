/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.assets_pack;

import lysa.log;
import lysa.virtual_fs;
import lysa.resources.render_target;

namespace lysa {

    std::vector<std::shared_ptr<vireo::Image>> AssetsPack::loadImagesAndTextures(
        const Header& header,
        std::vector<ImageTexture>& textures,
        const vireo::Buffer& stagingBuffer,
        const vireo::CommandList& commandList,
        std::ifstream &stream,
        const std::vector<ImageHeader>& imageHeaders,
        const std::vector<std::vector<MipLevelInfo>>&levelHeaders,
        const std::vector<TextureHeader>& textureHeaders) {
        const auto& vireo = ctx().vireo;
        std::vector<std::shared_ptr<vireo::Image>> images(header.texturesCount);

        // Create images upload buffer
        static constexpr size_t BLOCK_SIZE = 64 * 1024;
        auto blockSize = BLOCK_SIZE;
        auto transferBuffer = std::vector<char> (BLOCK_SIZE);
        auto transferOffset = size_t{0};
        while (stream.read(transferBuffer.data(), blockSize) || stream.gcount() > 0) {
            const auto bytesRead = stream.gcount();
            stagingBuffer.write(transferBuffer.data(), bytesRead, transferOffset);
            transferOffset += bytesRead;
            if ((transferOffset + blockSize) > stagingBuffer.getSize()) {
                blockSize = stagingBuffer.getSize() - transferOffset;
                if (blockSize == 0) {
                    break;
                }
            }
        }
        // std::printf("%lu bytes read\n", transferOffset);

        // Create all images from this upload buffer
        for (auto textureIndex = 0; textureIndex < header.texturesCount; ++textureIndex) {
            const auto& texture = textureHeaders[textureIndex];
            if (texture.imageIndex != -1) {
                const auto& imageHeader = imageHeaders[texture.imageIndex];
                // INFO("Loading image ", imageHeader.name, " ", imageHeader.width, "x", imageHeader.height, " ", imageHeader.format);
                // print(imageHeader);
                const auto& name = imageHeader.name;
                const auto image = vireo->createImage(
                    static_cast<vireo::ImageFormat>(imageHeader.format),
                    imageHeader.width,
                    imageHeader.height,
                    imageHeader.mipLevels,
                    1,
                    name);
                commandList.barrier(
                    image,
                    vireo::ResourceState::UNDEFINED,
                    vireo::ResourceState::COPY_DST,
                    0,
                    imageHeader.mipLevels);
                auto sourceOffsets = std::vector<size_t>(imageHeader.mipLevels);
                for (int mipLevel = 0; mipLevel < imageHeader.mipLevels; ++mipLevel) {
                    sourceOffsets[mipLevel] = imageHeader.dataOffset + levelHeaders[texture.imageIndex][mipLevel].offset;
                }
                commandList.copy(
                    stagingBuffer,
                    *image,
                    sourceOffsets);
                images[textureIndex] = image;
                // commandList.barrier(
                    // image,
                    // vireo::ResourceState::COPY_DST,
                    // vireo::ResourceState::SHADER_READ,
                    // 0,
                    // imageHeader.mipLevels);
                auto samplerIndex = ctx().samplers.addSampler(
                    static_cast<vireo::Filter>(texture.minFilter),
                    static_cast<vireo::Filter>(texture.magFilter),
                    static_cast<vireo::AddressMode>(texture.samplerAddressModeU),
                    static_cast<vireo::AddressMode>(texture.samplerAddressModeV));
                auto& lImage = ctx().res.get<ImageManager>().create(image, name);
                textures.push_back({lImage.id, samplerIndex});
            }
        }
        return images;
    }

}
