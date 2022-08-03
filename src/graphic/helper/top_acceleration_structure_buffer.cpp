#include "top_acceleration_structure_buffer.h"

#include "function_load.h"
#include "command_buffer.h"
#include "../device.h"


TopAccelerationStructureBuffer::TopAccelerationStructureBuffer(const Device* device)
:Buffer(), properties(), device(device),
structureGeometry(), buildSizeInfo(), accelerationStructure(VK_NULL_HANDLE),
instancesBuffer(device), acBuffer(device), acDeviceAddress(0), scratchBuffer(device) {}

TopAccelerationStructureBuffer::~TopAccelerationStructureBuffer() {
	FuncLoad::vkDestroyAccelerationStructureKHR(device->getDevice(), accelerationStructure, nullptr);
}

void TopAccelerationStructureBuffer::init() {
	createInstancesBuffer();
	getBuildSize();
	createScratchBuffer();
	createAccelerationStructureBuffer();
	buildAccelerationStructure();

	tlasWriteSetStructure = {};
	tlasWriteSetStructure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	tlasWriteSetStructure.accelerationStructureCount = 1;
	tlasWriteSetStructure.pAccelerationStructures = &accelerationStructure;
}

void TopAccelerationStructureBuffer::createInstancesBuffer() {
	instancesBuffer.properties.bufferSize = sizeof(VkAccelerationStructureInstanceKHR) * properties.blasInstances.size();
	instancesBuffer.properties.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	instancesBuffer.properties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	instancesBuffer.init();

	instancesBuffer.passData((void*) properties.blasInstances.data());
}

void TopAccelerationStructureBuffer::getBuildSize() {
	structureGeometry = {};
	structureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	structureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	structureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	structureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	structureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	structureGeometry.geometry.instances.data.deviceAddress = instancesBuffer.getAddress();

	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &structureGeometry;

	buildSizeInfo = {};
	buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	uint32_t primitive_count = properties.blasInstances.size();
	FuncLoad::vkGetAccelerationStructureBuildSizesKHR(
		device->getDevice(), 
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&primitive_count,
		&buildSizeInfo
	);
}

void TopAccelerationStructureBuffer::createScratchBuffer() {
	scratchBuffer.properties.bufferSize = buildSizeInfo.buildScratchSize;
	scratchBuffer.properties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	scratchBuffer.properties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	scratchBuffer.init();
}

void TopAccelerationStructureBuffer::createAccelerationStructureBuffer() {
	acBuffer.properties.bufferSize = buildSizeInfo.accelerationStructureSize;
	acBuffer.properties.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	acBuffer.properties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	acBuffer.init();
}

void TopAccelerationStructureBuffer::buildAccelerationStructure() {
	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = acBuffer.getBuffer();
	accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	FuncLoad::vkCreateAccelerationStructureKHR(device->getDevice(), &accelerationStructureCreateInfo, nullptr, &accelerationStructure);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = accelerationStructure;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &structureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getAddress();

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = properties.blasInstances.size();
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getComputeQueue());
	commandBuffer.start();

	FuncLoad::vkCmdBuildAccelerationStructuresKHR(
		commandBuffer.getCommandBuffer(), 1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data()
	);

	commandBuffer.end();

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = accelerationStructure;
	acDeviceAddress = FuncLoad::vkGetAccelerationStructureDeviceAddressKHR(device->getDevice(), &accelerationDeviceAddressInfo);
}

VkWriteDescriptorSet TopAccelerationStructureBuffer::getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	VkWriteDescriptorSet writeSet{};

	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = &tlasWriteSetStructure;
	writeSet.dstSet = descriptorSet;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	return writeSet;
}

VkDescriptorType TopAccelerationStructureBuffer::getDescriptorType() const {
	return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
}

const DataBuffer& TopAccelerationStructureBuffer::getBuffer() const {
	return acBuffer;
}

const VkAccelerationStructureKHR& TopAccelerationStructureBuffer::getAccelerationStructure() const {
	return accelerationStructure;
}

const VkDeviceAddress& TopAccelerationStructureBuffer::getAddress() const {
	return acDeviceAddress;
}
