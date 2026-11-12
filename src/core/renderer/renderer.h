#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <memory>

#include "rendererconfig.h"
#include "renderdata.h"
#include <vulkan/vulkan.h>

namespace Lumos
{
    /*
     * Forward declarations
     */
    class Window;
    class Device;
    class SwapChain;
    class Buffer;
    class Image;
    class Sampler;
    class GraphicsPipeline;

    class Renderer
    {
    public:
        /*
         * RendererConfig, configures app name, framesinflight, and wether validation layers are active
         */
        explicit Renderer(Window& window, const RendererConfig& config = {});
        ~Renderer();
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void render(const RenderData& renderdata);
        void waitIdle();
        void windowResized(bool resized);
        void cleanup();
        void setTexture(const Image& image, const Sampler& sampler);

        std::unique_ptr<Image> createImage(uint32_t width, uint32_t height);
        std::unique_ptr<Buffer> createBuffer(VkDeviceSize bufsize, void* data);
        std::unique_ptr<Sampler> createSampler();
        void transitionImageLayout(VkImage image,
                                   VkFormat format,
                                   VkImageLayout oldlayout,
                                   VkImageLayout newlayout);
        void copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height);

        // bool stenchilComponent(VkFormat format);
        // void copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);
        // void transitionImageLayout(VkImage image,
        //                            VkCommandBuffer cmdbuf,
        //                            VkImageLayout oldlayout,
        //                            VkImageLayout newlayout,
        //                            VkPipelineStageFlags2 srcstagemask,
        //                            VkPipelineStageFlags2 dststagemask,
        //                            VkAccessFlags2 srcaccessmask,
        //                            VkAccessFlags2 dstaccessmask,
        //                            VkImageAspectFlags aspectflags);
        // void transitionImageLayout2(VkImage image,
        //                             VkFormat format,
        //                             VkImageLayout oldlayout,
        //                             VkImageLayout newlayout);
        // VkSemaphore createSemaphore();

    private:
        bool beginFrame();
        void endFrame();
        void beginSwapchainRendering(VkCommandBuffer cmdbuf);
        void endSwapchainRendering(VkCommandBuffer cmdbuf);
        void transitioSwapchainImage(VkCommandBuffer cmdbuf, VkImageLayout oldlayout, VkImageLayout newlayout);
        void recordCommands(VkCommandBuffer cmdbuf, const RenderData& renderdata);
        void prepareFrame(VkCommandBuffer cmdbuf, const RenderData& renderdata);
        VkCommandBuffer currentCommandBuffer() const;
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer cmdbuf);
        void createCommandPools();
        void createCommandBuffers();
        void createSyncObjects();
        void createCameraBuffers();
        void createCameraSetLayout();
        void createDescriptorPool();
        void allocateCameraDescriptors();
        void updateCameraSets();
        void recreateSwapchain();
        //temp
        void updateTextureDescriptor(VkImageView imageview, VkSampler sampler);
        glm::mat4 viewproj(const CameraData& camera) const;
        Window& m_window;
        std::unique_ptr<Device> m_device;
        std::unique_ptr<SwapChain> m_swapchain;
        RendererConfig m_config;
        bool m_initialized = false;
        bool m_framebufferresized = false;
        bool m_framestarted = false;
        uint32_t m_frameindex = 0;
        uint32_t m_imageindex = 0;
        static constexpr uint32_t MAXENTITIES = 10000;

        //current frame data
        struct FrameData
        {
            VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
            VkSemaphore imageavailablesemaphore = VK_NULL_HANDLE;
            VkSemaphore renderfinishedsemaphore = VK_NULL_HANDLE;
            VkFence inflightfence = VK_NULL_HANDLE;
            std::unique_ptr<Buffer> camerabuffer;
            std::unique_ptr<Buffer> entitybuffer;
        };
        std::vector<FrameData> m_framedata;
        VkCommandPool m_commandpool = VK_NULL_HANDLE;

        struct CameraUBO
        {
            glm::mat4 viewproj = glm::mat4(1.0f);
        };
        VkDescriptorSetLayout m_camerasetlayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorpool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_camerasets;
        std::unique_ptr<GraphicsPipeline> m_defaultpipeline;

        VkDescriptorSetLayout m_entitysetlayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_entityset;

        VkDescriptorSetLayout m_texturelayout = VK_NULL_HANDLE;
        VkDescriptorSet m_textureset;

    };
} //namespace Lumos
