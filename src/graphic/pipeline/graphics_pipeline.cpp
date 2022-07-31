#include "graphics_pipeline.h"

#include "../device.h"
#include "../mesh.h"
#include "../graphics_object.h"
#include "../helper/descriptor.h"

#define VERTEX_SHADER "object_shader_vert.spv"
#define FRAGMENT_SHADER "object_shader_frag.spv"

#define GLOBAL_BINDING_SET_INDEX 0


GraphicsPipeline::GraphicsPipeline(Device* device)
:Pipeline(device) {}

GraphicsPipeline::~GraphicsPipeline() {
	vkDeviceWaitIdle(device->getDevice());
	
	vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->getDevice(), device->renderInfo.objectDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->getDevice(), device->renderInfo.globalDescriptorSetLayout, nullptr);
}

void GraphicsPipeline::init() {
	vertShaderCode = readFile(VERTEX_SHADER);
	fragShaderCode = readFile(FRAGMENT_SHADER);

	createDescriptorSetLayout(GraphicsPipeline::getUniformBindings(), &device->renderInfo.globalDescriptorSetLayout);
	createDescriptorSetLayout(GraphicsObject::getUniformBindings(), &device->renderInfo.objectDescriptorSetLayout);
	createGraphicsPipeline();
	createDescriptorPool();
	createDescriptors();
}

void GraphicsPipeline::updateUniforms(size_t index) {
	globalData.lightPosition = device->renderInfo.lightPosition;

	descriptors.at(index).getBuffer().passData((void*) &globalData);

	for (GraphicsObject* obj: objects) {
		obj->passBufferData(index);
	}
}

void GraphicsPipeline::recordRenderCommandBuffer(size_t index, const VkCommandBuffer* commandBuffer) {
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	descriptors.at(index).bind(commandBuffer);

	for (GraphicsObject* obj: objects) {
		obj->recordCommandBuffer(index, commandBuffer);
	}
}

void GraphicsPipeline::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& uniformBindings, VkDescriptorSetLayout* pSetLayout) {
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t) uniformBindings.size();
	layoutInfo.pBindings = uniformBindings.data();

	if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, pSetLayout) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorSetLayout", "failed to create descriptor set layout!");
	}
}

void GraphicsPipeline::createGraphicsPipeline() {
	VkShaderModule vertShaderModule = createShaderModule(device->getDevice(), vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(device->getDevice(), fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkVertexInputBindingDescription bindingDescription = Mesh::getBindingDescription();
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Mesh::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t) attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) device->renderInfo.swapchainExtend.width;
	viewport.height = (float) device->renderInfo.swapchainExtend.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = device->renderInfo.swapchainExtend;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	std::vector<VkDescriptorSetLayout> setLayouts {
		device->renderInfo.globalDescriptorSetLayout,
		device->renderInfo.objectDescriptorSetLayout,
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw InitException("vkCreatePipelineLayout", "failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = device->renderInfo.swapchainRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw InitException("vkCreateGraphicsPipelines", "failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device->getDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device->getDevice(), vertShaderModule, nullptr);
}

void GraphicsPipeline::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = device->renderInfo.swapchainImageCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = device->renderInfo.swapchainImageCount;

	if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorPool", "failed to create descriptor pool!");
	}
}

void GraphicsPipeline::createDescriptors() {
	descriptors.reserve(device->renderInfo.swapchainImageCount);

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		descriptors.emplace_back(device);

		descriptors.at(i).bindingSetIndex = GLOBAL_BINDING_SET_INDEX;
		descriptors.at(i).bufferSize = sizeof(GraphicsPipeline::GlobalData);
		descriptors.at(i).descriptorPool = descriptorPool;
		descriptors.at(i).setLayout = device->renderInfo.globalDescriptorSetLayout;

		descriptors.at(i).init();
	}
}

const VkPipelineLayout& GraphicsPipeline::getPipelineLayout() const {
	return pipelineLayout;
}

const VkPipeline& GraphicsPipeline::getGraphicsPipeline() const {
	return graphicsPipeline;
}

std::vector<VkDescriptorSetLayoutBinding> GraphicsPipeline::getUniformBindings() {
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings.at(0).binding         = 0;
	bindings.at(0).descriptorCount = 1;
	bindings.at(0).descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings.at(0).stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}

uint32_t GraphicsPipeline::getBindingSet() {
	return GLOBAL_BINDING_SET_INDEX;
}
