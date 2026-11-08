#pragma once
#include "renderer/descriptor.h"
#include "renderer/texture.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

namespace Lumos
{
    enum class BufferType
    {
        Uniform,
        Storage
    };

    class Device;
    class Buffer;
    class Texture;
    class Image;
    class ResourceManager
    {
    public:
        explicit ResourceManager(Device& device);
        ~ResourceManager() = default;

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) noexcept = default;
        ResourceManager& operator=(ResourceManager&&) noexcept = default;

        bool create(const std::string& name,
                    std::span<const PoolConfig> poolconfig,
                    std::span<const BindingConfig> bindings,
                    uint32_t maxsets);

        bool cacheBuffers(const std::string& name, std::vector<Buffer*> buffers);
        Buffer* getBufferByNameAndIndex(const std::string& name, uint32_t index) const;
        std::vector<Buffer*> createBuffer(const std::string& name, void* size, void* data, VkBufferUsageFlags type) const;

        bool createTexture(const std::string& name, std::filesystem::path filepath);
        /*
         * Samplers FilterMode is Linear and anistropy is enabled by default.
         * Probably should look into a way that those can be edited
         */
        bool createTextureImage(const std::string& name, uint32_t width, uint32_t height, VkFormat format);
        bool createTextureWithoutSampler(const std::string& name, uint32_t width, uint32_t height, VkFormat format, bool depth = false);
        Texture* getTextureByName(const std::string& name) const;

        bool allocDescriptorSets(const std::string& name, uint32_t size);
        VkDescriptorSet getSetByNameAndIndex(const std::string& name, uint32_t index) const;

        void createDescriptorLayout(const std::string& name, std::span<const BindingConfig> bindings);
        VkDescriptorSetLayout getLayoutByName(const std::string& name);

        void createDescriptorPool(const std::string& name, std::span<const PoolConfig> configs, uint32_t maxsets);
        VkDescriptorPool getPoolByName(const std::string& name);
        DescriptorPool* getPoolClassByName(const std::string& name);

    private:
        Device* m_device = nullptr;
        std::unordered_map<std::string, std::vector<Buffer*>> m_buffers;
        std::unordered_map<std::string, Texture*> m_textures;
        std::unordered_map<std::string, std::unique_ptr<DescriptorLayout>> m_descriptorlayouts;
        std::unordered_map<std::string, std::unique_ptr<DescriptorPool>> m_descriptorpools;
        std::unordered_map<std::string, std::vector<VkDescriptorSet>> m_descriptorsets;
    };
}
