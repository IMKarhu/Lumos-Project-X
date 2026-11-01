#pragma once
#include "renderer/descriptor.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif

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
    class CORE_API ResourceManager
    {
    public:
        explicit ResourceManager(Device& device);
        ~ResourceManager() = default;

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) noexcept = default;
        ResourceManager& operator=(ResourceManager&&) noexcept = default;

        bool cacheBuffers(const std::string& name, std::vector<Buffer*> buffers);
        Buffer* getBufferByNameAndIndex(const std::string& name, uint32_t index);

        bool createTexture(const std::string& name, std::filesystem::path filepath);
        Texture* getTextureByName(const std::string& name);

        bool allocDescriptorSets(const std::string& name, uint32_t size);
        VkDescriptorSet getSetByNameAndIndex(const std::string& name, uint32_t index);

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
