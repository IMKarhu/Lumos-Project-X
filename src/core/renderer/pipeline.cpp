#include "pipeline.h"
#include "errorcheck.h"
#include "device.h"
#include <fstream>
#include <utility>

namespace Lumos
{
    GraphicsPipeline::~GraphicsPipeline()
    {
        destroy();
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept
        : m_device(std::exchange(other.m_device, nullptr))
        , m_pipeline(std::exchange(other.m_pipeline, VK_NULL_HANDLE))
        , m_layout(std::exchange(other.m_layout, VK_NULL_HANDLE))
    {

    }
    
    GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept
    {
        if (&other == this) {
            return *this;
        }

        destroy();
        m_device = std::exchange(other.m_device, nullptr);
        m_pipeline = std::exchange(other.m_pipeline, VK_NULL_HANDLE);
        m_layout = std::exchange(other.m_layout, VK_NULL_HANDLE);
        return *this;
    }

    VkPipeline GraphicsPipeline::pipeline() const noexcept
    {
        return m_pipeline;
    }

    VkPipelineLayout GraphicsPipeline::layout() const noexcept
    {
        return m_layout;
    }

    GraphicsPipeline GraphicsPipeline::create(Device& device, const GraphicsPipelineDescription& desc)
    {
        GraphicsPipeline pipeline;
        pipeline.m_device = &device;

        VkPipelineVertexInputStateCreateInfo vertexinputinfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(desc.vertexbindings.size()),
            .pVertexBindingDescriptions = desc.vertexbindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.vertexattributes.size()),
            .pVertexAttributeDescriptions = desc.vertexattributes.data()
        };

        VkPipelineInputAssemblyStateCreateInfo inputassembly {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = desc.topology,
            .primitiveRestartEnable = VK_FALSE
        };

        std::vector<VkDynamicState> dynamicstates;
        if (desc.dynamicviewport) {
            dynamicstates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        }
        if (desc.dynamicscissor) {
            dynamicstates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        }
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
            .polygonMode = desc.polygonmode,
            .cullMode = desc.cullmode,
            .frontFace = desc.frontface,
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
            .dstAlphaBlendFactor = desc.dstalphablendfactor,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineDepthStencilStateCreateInfo depthstencil {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = desc.depthtest,
            .depthWriteEnable = desc.depthwrite,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {}, // Optional
            .back = {}, // Optional
            .minDepthBounds = 0.0f, // Optional
            .maxDepthBounds = 1.0f // Optional
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

        VkPipelineLayoutCreateInfo pipelinelayoutinfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            //these are only needed when push constants and descriptors are used
            .setLayoutCount = static_cast<uint32_t>(desc.descriptorlayouts.size()),
            .pSetLayouts = desc.descriptorlayouts.data(),
            .pushConstantRangeCount = desc.pushconstantcount,
            .pPushConstantRanges = desc.pushconstantranges.size() == 0 ? nullptr : desc.pushconstantranges.data()  
        };

        VK_CHECK(vkCreatePipelineLayout(device.device(), &pipelinelayoutinfo, nullptr, &pipeline.m_layout));

        //desc.depthformat when that is used
        VkPipelineRenderingCreateInfo renderinginfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &desc.colorformat
        };

        VkGraphicsPipelineCreateInfo pipelineinfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderinginfo,
            .stageCount = 2,
            .pStages = desc.shaderstages.data(),
            .pVertexInputState = &vertexinputinfo,
            .pInputAssemblyState = &inputassembly,
            .pViewportState = &viewportstate,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthstencil,
            .pColorBlendState = &colorblending,
            .pDynamicState = &dynamiccreateinfo,
            .layout = pipeline.m_layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // Optional
            .basePipelineIndex = -1 // Optional
        };

        VK_CHECK(vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &pipeline.m_pipeline));
        return pipeline;
    }

    void GraphicsPipeline::destroy()
    {
        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device->device(), m_pipeline, nullptr);
        }
        if (m_layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device->device(), m_layout, nullptr);
        }

        m_device = nullptr;
        m_pipeline = VK_NULL_HANDLE;
        m_layout = VK_NULL_HANDLE;
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

    VkShaderModule shaderModule(const std::vector<char>& buffer, VkDevice device)
    {
        VkShaderModuleCreateInfo moduleinfo{};
        moduleinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleinfo.codeSize = buffer.size();
        moduleinfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
        VkShaderModule shadermodule;
        VK_CHECK(vkCreateShaderModule(device, &moduleinfo, nullptr, &shadermodule));
        return shadermodule;
    }
} //namespace Lumos
