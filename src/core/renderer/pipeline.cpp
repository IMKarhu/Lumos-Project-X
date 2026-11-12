#include "pipeline.h"
#include "errorcheck.h"
#include "device.h"
#include "renderdata.h"
#include <fstream>
#include <utility>
#include <array>

namespace Lumos
{
    GraphicsPipeline::GraphicsPipeline(Device& device,
            VkDescriptorSetLayout camerasetlayout,
            VkDescriptorSetLayout entitysetlayout,
            VkDescriptorSetLayout texturesetlayout, 
            VkFormat colorformat)
        : m_device(device)
        , m_camerasetlayout(camerasetlayout)
        , m_entitysetlayout(entitysetlayout)
        , m_texturesetlayout(texturesetlayout)
        , m_colorformat(colorformat)
    {
        createPipelineLayout();
        createPipeline("shaders/sprite.spv");
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device.device(), m_layout, nullptr);
    }

    VkPipelineLayout GraphicsPipeline::layout() const noexcept
    {
        return m_layout;
    }

    void GraphicsPipeline::createPipeline(const std::string& shaderpath)
    {
        auto code = readFile(shaderpath);
        VkShaderModule shadermodule = shaderModule(code, m_device.device());

        VkPipelineShaderStageCreateInfo vertexShaderStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shadermodule,
            .pName = "vertMain",
        };

        VkPipelineShaderStageCreateInfo fragmentShaderStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shadermodule,
            .pName = "fragMain"
        };
        VkPipelineShaderStageCreateInfo stages[] = {
            vertexShaderStageCI,
            fragmentShaderStageCI
        };

        VkPipelineVertexInputStateCreateInfo vertexinputinfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
        };

        VkPipelineInputAssemblyStateCreateInfo inputassembly {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        std::vector<VkDynamicState> dynamicstates;
        dynamicstates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicstates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        VkPipelineDynamicStateCreateInfo dynamiccreateinfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicstates.size()),
            .pDynamicStates = dynamicstates.data()
        };

        VkPipelineViewportStateCreateInfo viewportstate {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };

        VkPipelineRasterizationStateCreateInfo rasterizer {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f, // Optional
            .depthBiasSlopeFactor = 0.0f, // Optional
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo multisampling {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f, // Optional
            .pSampleMask = nullptr, // Optional
            .alphaToCoverageEnable = VK_FALSE, // Optional
            .alphaToOneEnable = VK_FALSE // Optional
        };

        VkPipelineColorBlendAttachmentState colorblendattachment {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorblending {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = 1,
            .pAttachments = &colorblendattachment,
            .blendConstants = { // optional
                0.0f,
                0.0f,
                0.0f,
                0.0f
            }
        };

        VkPipelineRenderingCreateInfo renderinginfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_colorformat,
        };

        VkGraphicsPipelineCreateInfo pipelineinfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderinginfo,
            .stageCount = 2,
            .pStages = stages,
            .pVertexInputState = &vertexinputinfo,
            .pInputAssemblyState = &inputassembly,
            .pViewportState = &viewportstate,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorblending,
            .pDynamicState = &dynamiccreateinfo,
            .layout = m_layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // Optional
            .basePipelineIndex = -1 // Optional
        };

        VK_CHECK(vkCreateGraphicsPipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &m_pipeline));


        vkDestroyShaderModule(m_device.device(), shadermodule, nullptr);
    }

    void GraphicsPipeline::bind(VkCommandBuffer cmdbuf)
    {
        vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    void GraphicsPipeline::createPipelineLayout()
    {
        // VkPushConstantRange pushconstantrange{
        //     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        //     .offset = 0,
        //     .size = sizeof(ObjectPushConstant)
        // };

        std::array<VkDescriptorSetLayout, 3> layouts {m_camerasetlayout, m_entitysetlayout, m_texturesetlayout};
        VkPipelineLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data()
            // .pushConstantRangeCount = 1,
            // .pPushConstantRanges = &pushconstantrange
        };
        VK_CHECK(vkCreatePipelineLayout(m_device.device(), &info, nullptr, &m_layout));
    }


    VkShaderModule GraphicsPipeline::shaderModule(const std::vector<char>& buffer, VkDevice device)
    {
        VkShaderModuleCreateInfo moduleinfo{};
        moduleinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleinfo.codeSize = buffer.size();
        moduleinfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
        VkShaderModule shadermodule;
        VK_CHECK(vkCreateShaderModule(device, &moduleinfo, nullptr, &shadermodule));
        return shadermodule;
    }

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open shader file");
        }
        size_t filesize = (size_t)file.tellg();
        std::vector<char> buffer(filesize);
        file.seekg(0);
        file.read(buffer.data(), filesize);
        file.close();
        return buffer;
    }
} //namespace Lumos
