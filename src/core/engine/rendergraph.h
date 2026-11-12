#pragma once
#include <string>
#include <vector>
#include <functional>

#include <vulkan/vulkan.h>

namespace Lumos
{
    class Renderer;
    class RenderGraph
    {
    public:
        struct Resource
        {
            std::string name;
            VkFormat format;
            VkExtent2D extent;
            VkImageUsageFlags usageflags;
            VkImageLayout initiallayout;
            VkImageLayout finallayout;

            //gpu resources:
            VkImage image;
            VkImageView view;
            VkDeviceMemory imagememory;
        };

        explicit RenderGraph(Renderer& renderer);

        void addPass(const std::string& name,
                VkFormat format,
                VkExtent2D extent,
                VkImageUsageFlags usageflags,
                VkImageLayout initiallayout,
                VkImageLayout finallayout);
        void addPass(const std::string& name,
                std::vector<std::string> inputs,
                std::vector<std::string> outputs,
                std::function<void(VkCommandBuffer&)> func);

        void compile();

        void execute(VkCommandBuffer cmdbuf, VkQueue queue);

        Resource* getResource(const std::string& name);
    private:
        Renderer& m_renderer;

        struct Pass
        {
            std::string name;
            std::vector<std::string> inputs;
            std::vector<std::string> outputs;
            std::function<void(VkCommandBuffer&)> func; //rendering code
        };

        std::unordered_map<std::string, Resource> m_resources;
        std::vector<Pass> m_passes;
        std::vector<size_t> m_execorder;
        std::vector<VkSemaphore> m_semaphores;
        std::vector<std::pair<size_t, size_t>> m_sempahoresignalwaitpairs;
    };
} //namespace Lumos
