#include "renderer.h"
#include <stdexcept>
#include <vector>
#include <cstring>
#include <array>

#include "windowing/window.h"
#include "device.h"
#include "swapchain.h"
#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "pipeline.h"
#include "errorcheck.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Lumos
{
    Renderer::Renderer(Window& window, const RendererConfig& config)
        : m_window(window)
        , m_config(config)
    {
        m_device = std::make_unique<Device>(window, config);
        m_swapchain = std::make_unique<SwapChain>(*m_device, window, config);
        createCommandPools();
        createCommandBuffers();
        createSyncObjects();
        createCameraBuffers();
        createCameraSetLayout();
        createDescriptorPool();
        allocateCameraDescriptors();
        updateCameraSets();
        m_defaultpipeline = std::make_unique<GraphicsPipeline>(*m_device,
                m_camerasetlayout,
                m_entitysetlayout,
                m_texturelayout,
                m_swapchain->imageformat());
    }

    Renderer::~Renderer()
    {
        cleanup();
    }

    void Renderer::cleanup()
    {
        vkDestroyDescriptorPool(m_device->device(), m_descriptorpool, nullptr);
        for (size_t i = 0; i < m_framedata.size(); ++i) {
            vkDestroySemaphore(m_device->device(), m_framedata[i].imageavailablesemaphore, nullptr);
            vkDestroySemaphore(m_device->device(), m_framedata[i].renderfinishedsemaphore, nullptr);
            vkDestroyFence(m_device->device(), m_framedata[i].inflightfence, nullptr);
        }
        vkDestroyCommandPool(m_device->device(), m_commandpool, nullptr);
    }

    void Renderer::setTexture(const Image& image, const Sampler& sampler)
    {
        updateTextureDescriptor(image.imageview(), sampler.sampler());
    }

    std::unique_ptr<Image> Renderer::createImage(uint32_t width, uint32_t height)
    {
        //default constructed for now
        return std::make_unique<Image>(*m_device,
                width, height,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
    }

    std::unique_ptr<Buffer> Renderer::createBuffer(VkDeviceSize bufsize, void* data)
    {
        std::unique_ptr<Buffer> buf = std::make_unique<Buffer>(*m_device, bufsize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* datas = buf->map();
        std::memcpy(datas, data, bufsize);
        buf->unmap();
        return std::move(buf);
    }

    std::unique_ptr<Sampler> Renderer::createSampler()
    {
        SamplerConfig config{
            .minfilter = FilterMode::Nearest,
            .magfilter = FilterMode::Nearest,
            .addressmodeu = AddressMode::ClampToEdge,
            .addressmodev = AddressMode::ClampToEdge,
            .addressmodew = AddressMode::ClampToEdge,
            .enableanisotropy = false
        };
        return std::make_unique<Sampler>(*m_device, config);
    }

    void Renderer::transitionImageLayout(VkImage image,
            VkFormat format, VkImageLayout oldlayout, VkImageLayout newlayout)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands();
        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldlayout,
            .newLayout = newlayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VkPipelineStageFlags sourcestage;
        VkPipelineStageFlags destinationstage;
        if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourcestage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationstage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newlayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourcestage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationstage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }
        vkCmdPipelineBarrier(cmdbuf,
                             sourcestage, destinationstage,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        endSingleTimeCommands(cmdbuf);
    }

    void Renderer::copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands();
        VkBufferImageCopy region {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1}
        };
        vkCmdCopyBufferToImage(cmdbuf,
                               buffer.buffer(),
                               image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
        endSingleTimeCommands(cmdbuf);
    }

    void Renderer::render(const RenderData& renderdata)
    {
        beginFrame();
        // if (!beginFrame()) {
        //     return;
        // }
        prepareFrame(currentCommandBuffer(), renderdata);
        recordCommands(currentCommandBuffer(), renderdata);
        endFrame();
    }

    void Renderer::waitIdle()
    {
        vkDeviceWaitIdle(m_device->device());
    }

    void Renderer::windowResized(bool resized)
    {
        m_framebufferresized = resized;
    }

    // bool Renderer::stenchilComponent(VkFormat format)
    // {
    //     return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    // }

    // VkSemaphore Renderer::createSemaphore()
    // {
    //     VkSemaphore semaphore;
    //     VkSemaphoreCreateInfo semaphoreinfo{};
    //     semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    //     VK_CHECK(vkCreateSemaphore(m_device, &semaphoreinfo, nullptr, &semaphore));
    //     return semaphore;
    // }

    /*********PRIVATE*********/

    bool Renderer::beginFrame()
    {
        auto& frame = m_framedata[m_frameindex];
        VK_CHECK(vkWaitForFences(m_device->device(), 1, &frame.inflightfence, VK_TRUE, UINT64_MAX));
        VkResult result = vkAcquireNextImageKHR(m_device->device(),
                                                m_swapchain->swapchain(),
                                                UINT64_MAX,
                                                frame.imageavailablesemaphore,
                                                VK_NULL_HANDLE,
                                                &m_imageindex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || m_framebufferresized)
        {
            std::println("swapchain out of date\n");
            recreateSwapchain();
            // return false;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        VK_CHECK(vkResetCommandBuffer(frame.cmdbuf, 0));
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(frame.cmdbuf, &info));
        m_framestarted = true;
        return true;
    }

    void Renderer::endFrame()
    {
        auto& frame = m_framedata[m_frameindex];
        VK_CHECK(vkEndCommandBuffer(frame.cmdbuf));

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { frame.imageavailablesemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &frame.cmdbuf;
        VkSemaphore signalSemaphores[] = { frame.renderfinishedsemaphore };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkResetFences(m_device->device(), 1, &frame.inflightfence));

        VK_CHECK(vkQueueSubmit(m_device->graphicsQueue(), 1, &submitinfo, frame.inflightfence));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_swapchain->swapchain() };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &m_imageindex;
        presentinfo.pResults = nullptr;

        VkResult res = vkQueuePresentKHR(m_device->presentQueue(), &presentinfo);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_framebufferresized) {
            std::println("swapchain out of date, suboptimal or framebuffer was resized\n");
            recreateSwapchain();
        }
        else if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to present swapchain image");
        }
        m_framestarted = false;
        m_frameindex = (m_frameindex + 1) % m_config.framesinflight;
    }

    void Renderer::beginSwapchainRendering(VkCommandBuffer cmdbuf)
    {
        VkRenderingAttachmentInfo attachmentinfo{};
        attachmentinfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentinfo.imageView = m_swapchain->imageviews()[m_imageindex];
        attachmentinfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentinfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentinfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentinfo.clearValue.color = {0.1f, 0.1f, 0.1f, 1.0f};

        VkRenderingInfoKHR renderinfo{};
        renderinfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderinfo.renderArea = {.offset = {0, 0}, .extent = m_swapchain->extent()};
        renderinfo.layerCount = 1;
        renderinfo.colorAttachmentCount = 1;
        renderinfo.pColorAttachments = &attachmentinfo;

        vkCmdBeginRendering(cmdbuf, &renderinfo);

        uint32_t width = static_cast<float>(m_swapchain->extent().width);
        uint32_t height = static_cast<float>(m_swapchain->extent().height);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdbuf, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = {width, height};
        vkCmdSetScissor(cmdbuf, 0, 1, &scissor);
    }
    
    void Renderer::endSwapchainRendering(VkCommandBuffer cmdbuf)
    {
        vkCmdEndRendering(cmdbuf);
    }

    void Renderer::transitioSwapchainImage(VkCommandBuffer cmdbuf, VkImageLayout oldlayout, VkImageLayout newlayout)
    {
        VkImageMemoryBarrier2 barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.oldLayout = oldlayout;
        barrier.newLayout = newlayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_swapchain->images()[m_imageindex];
        barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        if (newlayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
            barrier.srcAccessMask = VK_ACCESS_2_NONE;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        } else if (newlayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
            barrier.dstAccessMask = VK_ACCESS_2_NONE;
        }

        VkDependencyInfo dependecyinfo {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(cmdbuf, &dependecyinfo);
    }

    void Renderer::recordCommands(VkCommandBuffer cmdbuf, const RenderData& renderdata)
    {
        transitioSwapchainImage(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        beginSwapchainRendering(cmdbuf);
        m_defaultpipeline->bind(cmdbuf);
        vkCmdBindDescriptorSets(cmdbuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_defaultpipeline->layout(),
                0, 1,
                &m_camerasets[m_frameindex],
                0,
                nullptr);
        vkCmdBindDescriptorSets(cmdbuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_defaultpipeline->layout(),
                1, 1,
                &m_entityset[m_frameindex],
                0,
                nullptr);
        vkCmdBindDescriptorSets(cmdbuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_defaultpipeline->layout(),
                2, 1,
                &m_textureset,
                0,
                nullptr);

        const uint32_t count = renderdata.renderobjects.size();
        if (count > 0) {
            vkCmdDraw(cmdbuf, 6, count, 0, 0);
        }
        endSwapchainRendering(cmdbuf);
        transitioSwapchainImage(cmdbuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    void Renderer::prepareFrame(VkCommandBuffer cmdbuf, const RenderData& renderdata)
    {
        std::vector<GpuEntity> m_entities;
        m_entities.reserve(renderdata.renderobjects.size());
        for (const auto& object : renderdata.renderobjects) {
            // const auto& object = renderdata.renderobjects.front();
            GpuEntity ent {
                .positionAndsize = {
                    object.position.x,
                    object.position.y,
                    object.size.x,
                    object.size.y
                }
            };
            ent.rotationdata.x = object.rotation;
            ent.color = object.color;
            ent.uvrect = object.uvrect;
            m_entities.push_back(ent);
        }
        if (m_entities.size() > MAXENTITIES) {
            throw std::runtime_error("Too many render entities. should probably separate them more");
        }

        if (!m_entities.empty()) {
            const VkDeviceSize bytes = m_entities.size() * sizeof(GpuEntity);
            auto& buffer = *m_framedata[m_frameindex].entitybuffer;
            std::memcpy(buffer.map(), m_entities.data(), bytes);
            buffer.unmap();
        }

        CameraUBO ubo{};
        ubo.viewproj = viewproj(renderdata.cameradata);
        auto& buffer = *m_framedata[m_imageindex].camerabuffer;
        std::memcpy(buffer.map(), &ubo, sizeof(CameraUBO));
        buffer.unmap();
    }

    VkCommandBuffer Renderer::currentCommandBuffer() const
    {
        if (!m_framestarted) {
            throw std::runtime_error("No frame currently active");
        }
        return m_framedata[m_frameindex].cmdbuf;
    }

    VkCommandBuffer Renderer::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocinfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandpool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VkCommandBuffer cmdbuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_device->device(), &allocinfo, &cmdbuffer));

        VkCommandBufferBeginInfo begininfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(vkBeginCommandBuffer(cmdbuffer, &begininfo));

        return cmdbuffer;
    }

    void Renderer::endSingleTimeCommands(VkCommandBuffer cmdbuf)
    {
        vkEndCommandBuffer(cmdbuf);
        VkSubmitInfo submitinfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdbuf
        };

        VK_CHECK(vkQueueSubmit(m_device->presentQueue(), 1, &submitinfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_device->presentQueue()));

        vkFreeCommandBuffers(m_device->device(), m_commandpool, 1, &cmdbuf);
    }

    
    void Renderer::createCommandPools() 
    {
        auto indices = m_device->findQueueFamilies(m_device->pdevice());
        VkCommandPoolCreateInfo poolinfo{};
        poolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolinfo.queueFamilyIndex = indices.graphicsFamily.value();
        VK_CHECK(vkCreateCommandPool(m_device->device(), &poolinfo, nullptr, &m_commandpool));

    }

    void Renderer::createCommandBuffers() 
    {
        m_framedata.resize(m_config.framesinflight);

        for (auto& data : m_framedata) {
            VkCommandBufferAllocateInfo allocinfo{};
            allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocinfo.commandPool = m_commandpool;
            allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocinfo.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(m_device->device(), &allocinfo, &data.cmdbuf));
        }
    }

    void Renderer::createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreinfo{};
        semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceinfo{};
        fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (auto& data : m_framedata) {
            VK_CHECK(vkCreateSemaphore(m_device->device(), &semaphoreinfo, nullptr, &data.imageavailablesemaphore));
            VK_CHECK(vkCreateSemaphore(m_device->device(), &semaphoreinfo, nullptr, &data.renderfinishedsemaphore));
            VK_CHECK(vkCreateFence(m_device->device(), &fenceinfo, nullptr, &data.inflightfence));
        }
    }

    void Renderer::createCameraBuffers()
    {
        const VkDeviceSize buffersize = sizeof(CameraUBO);
        for (auto& data : m_framedata) {
            data.camerabuffer = std::make_unique<Buffer>(*m_device,
                    buffersize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            data.entitybuffer = std::make_unique<Buffer>(*m_device,
                    sizeof(GpuEntity) * MAXENTITIES,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void Renderer::createCameraSetLayout()
    {
        VkDescriptorSetLayoutBinding binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        };
        VkDescriptorSetLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &binding
        };

        VK_CHECK(vkCreateDescriptorSetLayout(m_device->device(), &info, nullptr, &m_camerasetlayout));

        VkDescriptorSetLayoutBinding storagebinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        };
        VkDescriptorSetLayoutCreateInfo storageinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &storagebinding
        };
        VK_CHECK(vkCreateDescriptorSetLayout(m_device->device(), &storageinfo, nullptr, &m_entitysetlayout));

        VkDescriptorSetLayoutBinding texturebinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        };
        VkDescriptorSetLayoutBinding samplerbinding {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        };
        const std::array<VkDescriptorSetLayoutBinding, 2> bindings {
            texturebinding,
            samplerbinding
        };

        VkDescriptorSetLayoutCreateInfo texturesamplerinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };
        VK_CHECK(vkCreateDescriptorSetLayout(m_device->device(), &texturesamplerinfo, nullptr, &m_texturelayout));
    }

    void Renderer::updateCameraSets()
    {
        for (size_t i = 0; i < m_framedata.size(); ++i) {
            VkDescriptorBufferInfo bufinfo {
                .buffer = m_framedata[i].camerabuffer->buffer(),
                .offset = 0,
                .range = sizeof(CameraUBO)
            };
            VkWriteDescriptorSet write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_camerasets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufinfo
            };
            vkUpdateDescriptorSets(m_device->device(), 1, &write, 0, nullptr);

            VkDescriptorBufferInfo storageinfo {
                .buffer = m_framedata[i].entitybuffer->buffer(),
                .offset = 0,
                .range = VK_WHOLE_SIZE
            };
            VkWriteDescriptorSet entitywrite {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_entityset[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &storageinfo

            };
            vkUpdateDescriptorSets(m_device->device(), 1, &entitywrite, 0, nullptr);
        }
    }

    void Renderer::recreateSwapchain()
    {
        std::println("recreate called");
        VK_CHECK(vkDeviceWaitIdle(m_device->device()));
        auto replacement = std::make_unique<SwapChain>(*m_device, m_window, m_config, m_swapchain->swapchain());
        m_swapchain = std::move(replacement);
        m_framebufferresized = false;
    }

    void Renderer::createDescriptorPool()
    {
        std::println("create descriptor pool");
        std::array<VkDescriptorPoolSize, 4> poolsizes{};
        poolsizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolsizes[0].descriptorCount = static_cast<uint32_t>(m_framedata.size());
        poolsizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolsizes[1].descriptorCount = static_cast<uint32_t>(m_framedata.size());
        poolsizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        poolsizes[2].descriptorCount = 1;
        poolsizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        poolsizes[3].descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(m_framedata.size() * 2 + 1),
            .poolSizeCount = static_cast<uint32_t>(poolsizes.size()),
            .pPoolSizes = poolsizes.data()
        };

        VK_CHECK(vkCreateDescriptorPool(m_device->device(), &poolinfo, nullptr, &m_descriptorpool));
    }

    void Renderer::allocateCameraDescriptors()
    {
        std::vector<VkDescriptorSetLayout> layouts(m_framedata.size(), m_camerasetlayout);
        m_camerasets.resize(m_framedata.size());
        VkDescriptorSetAllocateInfo allocinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorpool,
            .descriptorSetCount = static_cast<uint32_t>(m_camerasets.size()),
            .pSetLayouts = layouts.data()
        };
        VK_CHECK(vkAllocateDescriptorSets(m_device->device(), &allocinfo, m_camerasets.data()));

        std::vector<VkDescriptorSetLayout> entitylayouts(m_framedata.size(), m_entitysetlayout);
        m_entityset.resize(m_framedata.size());
        VkDescriptorSetAllocateInfo entityallocinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorpool,
            .descriptorSetCount = static_cast<uint32_t>(m_entityset.size()),
            .pSetLayouts = entitylayouts.data()
        };
        VK_CHECK(vkAllocateDescriptorSets(m_device->device(), &entityallocinfo, m_entityset.data()));

        VkDescriptorSetAllocateInfo textureallocinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorpool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_texturelayout
        };
        VK_CHECK(vkAllocateDescriptorSets(m_device->device(), &textureallocinfo, &m_textureset));
    }

    glm::mat4 Renderer::viewproj(const CameraData& camera) const
    {
        const glm::vec2 halfsize = camera.size / (2.0f * camera.zoom);
        const glm::mat4 projection = glm::ortho(
                camera.position.x - halfsize.x,
                camera.position.x + halfsize.x,
                camera.position.y - halfsize.y,
                camera.position.y + halfsize.y,
                -1.0f,
                 1.0f);
        glm::mat4 view = glm::rotate(
                glm::mat4(1.0f),
                -camera.rotation,
                glm::vec3(0.0f, 0.0f, 1.0f));

        view = view * glm::translate(
                glm::mat4(1.0f),
                glm::vec3(-camera.position,
                0.0f));
        return projection * view;
    }

    void Renderer::updateTextureDescriptor(VkImageView imageview, VkSampler sampler)
    {
        // std::println("update texture set");
        VkDescriptorImageInfo imageinfo {
            .imageView = imageview,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkDescriptorImageInfo samplerinfo {
            .sampler = sampler
        };
        std::array<VkWriteDescriptorSet, 2> writes{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = m_textureset;
        writes[0].dstBinding = 0;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writes[0].pImageInfo = &imageinfo;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = m_textureset;
        writes[1].dstBinding = 1;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writes[1].pImageInfo = &samplerinfo;
        vkUpdateDescriptorSets(m_device->device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

} // namespace Lumos
