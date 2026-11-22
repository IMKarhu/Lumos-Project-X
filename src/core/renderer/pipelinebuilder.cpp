#include "pipelinebuilder.h"
#include "device.h"
#include "errorcheck.h"

namespace Lumos
{
    struct ScopedModule {
        explicit ScopedModule(VkDevice device) : m_device(device) {}
        ~ScopedModule()
        {
            if (m_module) {
                vkDestroyShaderModule(m_device, m_module, nullptr);
            }
        }
        VkDevice m_device;
        VkShaderModule m_module = VK_NULL_HANDLE;
    };

    VkResult shaderModule(VkDevice device, const std::span<const uint32_t> buffer, VkShaderModule& module)
    {
        VkShaderModuleCreateInfo moduleinfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = buffer.size_bytes(),
            .pCode = buffer.data()
        };
        VkShaderModule shadermodule;
        return vkCreateShaderModule(device, &moduleinfo, nullptr, &module);
    }
    
    std::expected<std::unique_ptr<GraphicsPipeline>, VkResult>
    PipelineBuilder::build(Device& device, VkPipelineCache cache) const
    {
        if (m_shader.empty()) {
            return std::unexpected(VK_ERROR_INITIALIZATION_FAILED);
        }
        ScopedModule scopedmodule{device.device()};
        VK_CHECK(shaderModule(device.device(), m_shader, scopedmodule.m_module));

        VkPipelineShaderStageCreateInfo vertexShaderStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = scopedmodule.m_module,
            .pName = "vertMain",
        };

        VkPipelineShaderStageCreateInfo fragmentShaderStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = scopedmodule.m_module,
            .pName = "fragMain"
        };
        std::vector<VkPipelineShaderStageCreateInfo> stages = {
            vertexShaderStageCI,
            fragmentShaderStageCI
        };

        VkPipelineVertexInputStateCreateInfo vertexinputinfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindings.size()),
            .pVertexBindingDescriptions = m_bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributes.size()),
            .pVertexAttributeDescriptions = m_attributes.data()
        };

        VkPipelineInputAssemblyStateCreateInfo inputassembly {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = m_topology,
            .primitiveRestartEnable = VK_FALSE
        };

        std::vector<VkDynamicState> dynamicstates;
        dynamicstates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicstates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        if (m_depthbias) {
            dynamicstates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
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
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = m_cull,
            .frontFace = m_frontface,
            .depthBiasEnable = m_depthbias,
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

        VkPipelineDepthStencilStateCreateInfo depthsampling {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = m_depthtest,
            .depthWriteEnable = m_depthwrite,
            .depthCompareOp = m_depthop
        };

        VkPipelineColorBlendAttachmentState colorblendattachment {
            .blendEnable = m_blend,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        std::vector<VkPipelineColorBlendAttachmentState> blendattachments(m_colorformats.size(), colorblendattachment);
        VkPipelineColorBlendStateCreateInfo colorblending {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = static_cast<uint32_t>(m_colorformats.size()),
            .pAttachments = blendattachments.data(),
            .blendConstants = { // optional
                0.0f,
                0.0f,
                0.0f,
                0.0f
            }
        };

        VkPipelineRenderingCreateInfo renderinginfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = static_cast<uint32_t>(m_colorformats.size()),
            .pColorAttachmentFormats = m_colorformats.data(),
            .depthAttachmentFormat = m_depthformat
        };

        /*
         * layout
         */
        VkPipelineLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(m_setlayouts.size()),
            .pSetLayouts = m_setlayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(m_pushranges.size()),
            .pPushConstantRanges = m_pushranges.data()
        };
        VkPipelineLayout layout = VK_NULL_HANDLE;
        if(!VK_CHECK(vkCreatePipelineLayout(device.device(), &info, nullptr, &layout))) {
            //returning VK_NOT_READY definitely isnt correct, but since VK_CHECK already logs
            //the error value I have no idea what to return here.
            //should probably look into making this actually return the correct value
            //and maybe I could log that value wherever its called instead of in vk_check
            return std::unexpected(VK_NOT_READY);
        }

        VkGraphicsPipelineCreateInfo pipelineinfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderinginfo,
            .stageCount = static_cast<uint32_t>(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertexinputinfo,
            .pInputAssemblyState = &inputassembly,
            .pViewportState = &viewportstate,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthsampling,
            .pColorBlendState = &colorblending,
            .pDynamicState = &dynamiccreateinfo,
            .layout = layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // Optional
            .basePipelineIndex = -1 // Optional
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        if(!VK_CHECK(vkCreateGraphicsPipelines(device.device(), cache, 1, &pipelineinfo, nullptr, &pipeline))) {
            vkDestroyPipelineLayout(device.device(), layout, nullptr);
            //same thing than in layout
            return std::unexpected(VK_NOT_READY);
        }

        return std::make_unique<GraphicsPipeline>(device.device(), pipeline, layout);
    }

} // namespace Lumos
