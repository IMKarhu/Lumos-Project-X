#include "resourcemanager.h"
#include "renderer/device.h"
#include "renderer/buffer.h"
#include "renderer/image.h"
#include "renderer/sampler.h"
#include "renderer/texture.h"
#include "ktx.h"
#include <vulkan/vulkan.h>
#include <cassert>
#include <cstring>
#include <print>

namespace Lumos
{
    ResourceManager::ResourceManager(Device& device)
        : m_device(&device)
    {

    }

    /*
     * Create pool, layout, allocate descriptor sets
     */
    bool ResourceManager::create(const std::string& name,
                                 std::span<const PoolConfig> poolconfig,
                                 std::span<const BindingConfig> bindings,
                                 uint32_t maxsets)
    {
        //pool
        auto exists = getPoolClassByName(name);
        if (exists) {
            return false;
        }
        auto pool = std::make_unique<DescriptorPool>(*m_device, poolconfig, maxsets);

        //layout
        auto layoutexists = getLayoutByName(name);
        if (layoutexists) {
            return false;
        }
        auto layout = std::make_unique<DescriptorLayout>(*m_device, bindings);


        std::vector<VkDescriptorSetLayout> layouts(maxsets, layout->layout());
        auto sets = pool->allocateDescriptorSets(layouts, maxsets);
        auto [it, result] = m_descriptorsets.emplace(name, sets);


        m_descriptorpools[name] = std::move(pool);
        m_descriptorlayouts[name] = std::move(layout);
        return result;
    }
     
    /*
     * inserts elements into a unordered map where key is the name, and value is Buffer
     * returns bool wether it was success or not.
     */
    bool ResourceManager::cacheBuffers(const std::string& name, std::vector<Buffer*> buffers)
    {
        auto [it, result] = m_buffers.emplace(name, buffers);
        std::println("caching buffer: {}", name);
        return result;
    }

    Buffer* ResourceManager::getBufferByNameAndIndex(const std::string& name, uint32_t index) const
    {
        auto it = m_buffers.find(name);
        if (it != m_buffers.end()) {
            return it->second[index];
        }
        // std::println("returning null");
        // just returning nullptr isnt really ideal. because whenever buffer is then used application may crash.
        return nullptr;
    }

    std::vector<Buffer*> ResourceManager::createBuffer(const std::string& name, void* size, void* data, VkBufferUsageFlags type) const
    {
        VkDeviceSize buffersize = (VkDeviceSize)size;
        std::vector<Buffer*> buffers;

        Buffer stagingbuffer(*m_device,
                             buffersize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        std::memcpy(stagingbuffer.map(), data, buffersize);
        stagingbuffer.unmap();

        Buffer* buffer = new Buffer(*m_device,
                       buffersize,
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | type,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_device->copyBuffers(stagingbuffer, *buffer, buffersize);
        buffers.push_back(buffer);
        // m_buffers[name] = buffers;
        return buffers;
    }

    bool ResourceManager::createTexture(const std::string& name, std::filesystem::path filepath)
    {
        ktxTexture* texture = nullptr;
        const KTX_error_code ktxresult = ktxTexture_CreateFromNamedFile(filepath.string().c_str(),
                KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                &texture);

        if (ktxresult != KTX_SUCCESS) {
            std::println("failed to load ktx texture: {}", ktxErrorString(ktxresult));
            return false;
        }
        std::println("texture size: {}x{}", texture->baseWidth, texture->baseHeight);
        uint32_t width = texture->baseWidth;
        uint32_t height = texture->baseHeight;
        ktx_size_t imagesize = ktxTexture_GetImageSize(texture, 0);
        ktx_uint8_t* texturedata = ktxTexture_GetData(texture);
        if (!texturedata) {
            std::println("data is null");
        }

        Buffer stagingbuffer(*m_device,
                imagesize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data = stagingbuffer.map();
        std::memcpy(data, texturedata, imagesize);
        stagingbuffer.unmap();

        Image image = Image(*m_device,
                            width,
                            height,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            VK_IMAGE_ASPECT_COLOR_BIT);
        SamplerConfig config{
            .filtermode = FilterMode::Nearest,
            .enableanisotropy = false
        };
        Sampler sampler = Sampler(*m_device, config);

        auto cmdbuf = m_device->beginSingleTimeCommands();
        m_device->transitionImageLayout2(image.image(),
                                         VK_FORMAT_R8G8B8A8_SRGB,
                                         VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
 
        m_device->copyBufferToImage(stagingbuffer, image.image(), width, height);
        m_device->transitionImageLayout2(image.image(),
                               VK_FORMAT_R8G8B8A8_SRGB,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        ktxTexture_Destroy(texture);
        auto [it, result] = m_textures.emplace(name, new Texture(std::move(image), std::move(sampler)));
        return result;
    }

    bool ResourceManager::createTextureImage(const std::string& name, uint32_t width, uint32_t height, VkFormat format)
    {
// VK_FORMAT_R8G8B8A8_SRGB
        Image image = Image(*m_device,
                width, height, format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

        SamplerConfig config{
            .filtermode = FilterMode::Linear,
            .enableanisotropy = true
        };
        Sampler sampler = Sampler(*m_device, config);

        auto [it, result] = m_textures.emplace(name, new Texture(std::move(image), std::move(sampler)));
        return result;
    }

    bool ResourceManager::createTextureWithoutSampler(const std::string& name, uint32_t width, uint32_t height, VkFormat format, bool depth)
    {
        VkImageUsageFlags usageflags;
        VkImageAspectFlags aspectflags;
        if (depth) {
            usageflags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            aspectflags = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else {
            usageflags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            aspectflags = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        Image image = Image(*m_device,
                width, height, format,
                VK_IMAGE_TILING_OPTIMAL,
                usageflags,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                aspectflags);
        auto [it, result] = m_textures.emplace(name, new Texture(std::move(image), {}));
        return result;
    }


    Texture* ResourceManager::getTextureByName(const std::string& name) const
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool ResourceManager::allocDescriptorSets(const std::string& name, uint32_t size)
    {
        std::vector<VkDescriptorSetLayout> layouts(size, getLayoutByName(name));
        auto pool = getPoolClassByName(name);
        auto sets = pool->allocateDescriptorSets(layouts, size);
        auto [it, result] = m_descriptorsets.emplace(name, sets);
        return result;
    }

    VkDescriptorSet ResourceManager::getSetByNameAndIndex(const std::string& name, uint32_t index) const
    {
        auto it = m_descriptorsets.find(name);
        if (it != m_descriptorsets.end()) {
            return it->second[index];
        }
        return nullptr;
    }

    void ResourceManager::createDescriptorLayout(const std::string& name, std::span<const BindingConfig> bindings)
    {
        auto exists = getLayoutByName(name);
        if (exists) {
            return;
        }
        auto layout = std::make_unique<DescriptorLayout>(*m_device, bindings);
        m_descriptorlayouts[name] = std::move(layout);

    }

    VkDescriptorSetLayout ResourceManager::getLayoutByName(const std::string& name)
    {
        auto it = m_descriptorlayouts.find(name);
        if (it != m_descriptorlayouts.end()) {
            return it->second->layout();
        }
        return nullptr;
    }

    void ResourceManager::createDescriptorPool(const std::string& name, std::span<const PoolConfig> configs, uint32_t maxsets)
    {
        auto exists = getPoolClassByName(name);
        if (exists) {
            return;
        }
        auto pool = std::make_unique<DescriptorPool>(*m_device, configs, maxsets);
        m_descriptorpools[name] = std::move(pool);
    }

    VkDescriptorPool ResourceManager::getPoolByName(const std::string& name)
    {
        assert(0 && "TODO: IMPLEMENT getPoolByName");
        return nullptr;
    }

    DescriptorPool* ResourceManager::getPoolClassByName(const std::string& name)
    {
        auto it = m_descriptorpools.find(name);
        if (it != m_descriptorpools.end()) {
            return it->second.get();
        }
        return nullptr;
    }

} //namespace Lumos
