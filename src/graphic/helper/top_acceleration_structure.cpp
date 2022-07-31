#include "top_acceleration_structure.h"

#include "function_load.h"
#include "command_buffer.h"
#include "../device.h"


TopAccelerationStructure::TopAccelerationStructure(const Device* device)
:blasInstances(), device(device),
structureGeometry(), buildSizeInfo(), accelerationStructure(VK_NULL_HANDLE),
instancesBuffer(device), acBuffer(device), acDeviceAddress(0), scratchBuffer(device) {}

TopAccelerationStructure::~TopAccelerationStructure() {
	FuncLoad::vkDestroyAccelerationStructureKHR(device->getDevice(), accelerationStructure, nullptr);
}

void TopAccelerationStructure::init() {
	createInstancesBuffer();
	getBuildSize();
	createScratchBuffer();
	createAccelerationStructureBuffer();
	buildAccelerationStructure();
}

void TopAccelerationStructure::createInstancesBuffer() {
	instancesBuffer.bufferSize = sizeof(VkAccelerationStructureInstanceKHR) * blasInstances.size();
	instancesBuffer.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	instancesBuffer.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	instancesBuffer.init();

	instancesBuffer.passData((void*) blasInstances.data());
}

void TopAccelerationStructure::getBuildSize() {
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

	uint32_t primitive_count = blasInstances.size();
	FuncLoad::vkGetAccelerationStructureBuildSizesKHR(
		device->getDevice(), 
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&primitive_count,
		&buildSizeInfo
	);
}

void TopAccelerationStructure::createScratchBuffer() {
	scratchBuffer.bufferSize = buildSizeInfo.buildScratchSize;
	scratchBuffer.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	scratchBuffer.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	scratchBuffer.init();
}

void TopAccelerationStructure::createAccelerationStructureBuffer() {
	acBuffer.bufferSize = buildSizeInfo.accelerationStructureSize;
	acBuffer.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	acBuffer.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	acBuffer.init();
}

void TopAccelerationStructure::buildAccelerationStructure() {
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
	accelerationStructureBuildRangeInfo.primitiveCount = blasInstances.size();
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

const Buffer& TopAccelerationStructure::getBuffer() const {
	return acBuffer;
}

const VkAccelerationStructureKHR& TopAccelerationStructure::getAccelerationStructure() const {
	return accelerationStructure;
}

const VkDeviceAddress& TopAccelerationStructure::getAddress() const {
	return acDeviceAddress;
}
