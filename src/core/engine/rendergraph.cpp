#include "rendergraph.h"
#include "renderer/renderer.h"
#include "renderer/errorcheck.h"
#include <stdexcept>

namespace Lumos
{
    RenderGraph::RenderGraph(Renderer& renderer)
        : m_renderer(renderer) {}

    void RenderGraph::addPass(const std::string& name, VkFormat format, VkExtent2D extent,
                VkImageUsageFlags usageflags, VkImageLayout initiallayout, VkImageLayout finallayout)
    {
        Resource res {
            .name = name,
            .format = format,
            .extent = extent,
            .usageflags = usageflags,
            .initiallayout = initiallayout,
            .finallayout = finallayout
        };
        m_resources[name] = res;
    }

    void RenderGraph::addPass(const std::string& name,
            std::vector<std::string> inputs,
            std::vector<std::string> outputs,
            std::function<void(VkCommandBuffer&)> func)
    {
        Pass pass {
            .name = name,
            .inputs = inputs,
            .outputs = outputs,
            .func = func
        };
        m_passes.push_back(pass);
    }

    void RenderGraph::compile()
    {
        std::vector<std::vector<size_t>> dependencies(m_passes.size());
        std::vector<std::vector<size_t>> dependents(m_passes.size());

        std::unordered_map<std::string, size_t> resourceWriters;

        for (size_t i = 0; i < m_passes.size(); ++i) {
            for (const auto& output : m_passes[i].outputs) {
                resourceWriters[output] = i;
            }
        }

        for (size_t i = 0; i < m_passes.size(); ++i) {
            const auto& pass = m_passes[i];

            for (const auto& input : pass.inputs) {
                auto it = resourceWriters.find(input);
                if (it != resourceWriters.end()) {
                    dependencies[i].push_back(it->second);
                    dependents[it->second].push_back(i);
                }
            }
        }

        std::vector<bool> visited(m_passes.size(), false);
        std::vector<bool> instack(m_passes.size(), false);

        std::function<void(size_t)> visit = [&](size_t node) {
            if (instack[node]) {
                throw std::runtime_error("Circular dependency found in rendergraph");
            }

            if (visited[node]) {
                return;
            }

            instack[node] = true;

            for (auto dependency : dependencies[node]) {
                visit(dependency);
            }

            instack[node] = false; //remove from path
            visited[node] = true; // processed
            m_execorder.push_back(node);
        };

        for (size_t i = 0; i < m_passes.size(); ++i) {
            if (!visited[i]) {
                visit(i);
            }
        }

        for (size_t i = 0; i < m_passes.size(); ++i) {
            for (auto dependency : dependencies[i]) {
                m_semaphores.emplace_back(m_renderer.createSemaphore());
                m_sempahoresignalwaitpairs.emplace_back(dependency, i);
            }
        }
        
        for (auto& [name, resource] : m_resources) {
            VkImageCreateInfo info {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = resource.format,
                .extent = {resource.extent.width, resource.extent.height, 1},
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = resource.usageflags,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
            };
            VK_CHECK(vkCreateImage(m_renderer.device(), &info, nullptr, &resource.image));

            VkMemoryRequirements memrequirements;
            vkGetImageMemoryRequirements(m_renderer.device(), resource.image, &memrequirements);
            VkMemoryAllocateInfo alloinfo {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memrequirements.size,
                .memoryTypeIndex = m_renderer.findMemoryType(memrequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            };
            VK_CHECK(vkAllocateMemory(m_renderer.device(), &alloinfo, nullptr, &resource.imagememory));
            vkBindImageMemory(m_renderer.device(), resource.image, resource.imagememory, 0);

            //image view
            VkImageViewCreateInfo imageviewinfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = resource.image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = resource.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };
            
            VK_CHECK(vkCreateImageView(m_renderer.device(), &imageviewinfo, nullptr, &resource.view));
        }
    }

    void RenderGraph::execute(VkCommandBuffer cmdbuf, VkQueue queue)
    {
        std::vector<VkCommandBuffer> cmdbuffers;
        std::vector<VkSemaphore> semaphores;
        std::vector<VkPipelineStageFlags> waitstages;
        std::vector<VkSemaphore> signalsemaphores;

        for (auto passindex : m_execorder) {
            const auto& pass = m_passes[passindex];

            semaphores.clear();
            waitstages.clear();

            for (size_t i = 0; i < m_sempahoresignalwaitpairs.size(); ++i) {
                if (m_sempahoresignalwaitpairs[i].second == passindex) {
                    semaphores.push_back(m_semaphores[i]);
                    waitstages.push_back(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
                }
            }
            signalsemaphores.clear();
            for (size_t i = 0; i < m_sempahoresignalwaitpairs.size(); ++i) {
                if (m_sempahoresignalwaitpairs[i].first == passindex) {
                    signalsemaphores.push_back(m_semaphores[i]);
                }
            }

            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VK_CHECK(vkBeginCommandBuffer(cmdbuf, &info));

            for (const auto& input : pass.inputs) {
                auto& resource = m_resources[input];
                //transition input resources
                //image barrier
                //pipeline barrier?
            }

            for (const auto& output : pass.outputs) {
                auto& resource = m_resources[output];
                //transition resources to render target layout
                //image barrier
                //pipelinebarrier
            }

            pass.func(cmdbuf);

            for (const auto& output : pass.outputs) {
                auto& resource = m_resources[output];
                //transition resources to render target layout
                //image barrier
                //pipelinebarrier
            }

            vkEndCommandBuffer(cmdbuf);

            //submit to queue

        }
    }

    RenderGraph::Resource* RenderGraph::getResource(const std::string& name)
    {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            return &it->second;
        }
        return nullptr;
    }

} // namespace Lumos
