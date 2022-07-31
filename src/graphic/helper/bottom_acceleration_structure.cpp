#include "bottom_acceleration_structure.h"

#include "function_load.h"
#include "command_buffer.h"
#include "../device.h"


BottomAccelerationStructure::BottomAccelerationStructure(const Device* device)
:device(device),
structureGeometry(), buildSizeInfo(), accelerationStructure(VK_NULL_HANDLE),
acBuffer(device), scratchBuffer(device) {}

BottomAccelerationStructure::~BottomAccelerationStructure() {
	FuncLoad::vkDestroyAccelerationStructureKHR(device->getDevice(), accelerationStructure, nullptr);
}

void BottomAccelerationStructure::init() {
	createStructureGeometry();
	getBuildSize();
	createAccelerationStructure();
	createScratchBuffer();
	computeAccelerationStructure();
}

void BottomAccelerationStructure::createStructureGeometry() {
	structureGeometry = {};
	structureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	structureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	structureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

	structureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;

	structureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	structureGeometry.geometry.triangles.vertexData.deviceAddress = vertexAddress;
	structureGeometry.geometry.triangles.vertexStride = vertexStride;
	structureGeometry.geometry.triangles.maxVertex = vertexCount;

	structureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	structureGeometry.geometry.triangles.indexData.deviceAddress = indexAddress;

	structureGeometry.geometry.triangles.transformData.deviceAddress = 0;
	structureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
}

void BottomAccelerationStructure::getBuildSize() {
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &structureGeometry;

	buildSizeInfo = {};
	buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	uint32_t triangleCount = indexCount / 3;
	FuncLoad::vkGetAccelerationStructureBuildSizesKHR(
		device->getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR,
		&buildInfo,
		&triangleCount,
		&buildSizeInfo
	);
}

void BottomAccelerationStructure::createAccelerationStructure() {
	acBuffer.bufferSize = buildSizeInfo.accelerationStructureSize;
	acBuffer.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	acBuffer.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	acBuffer.init();

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = acBuffer.getBuffer();
	accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	if (FuncLoad::vkCreateAccelerationStructureKHR(device->getDevice(), &accelerationStructureCreateInfo, nullptr, &accelerationStructure) != VK_SUCCESS) {
		throw InitException("vkCreateAccelerationStructureKHR", "could not create acceleration structure!");
	}
}

void BottomAccelerationStructure::createScratchBuffer() {
	scratchBuffer.bufferSize = buildSizeInfo.buildScratchSize;
	scratchBuffer.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	scratchBuffer.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	scratchBuffer.init();
}

void BottomAccelerationStructure::computeAccelerationStructure() {
	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	accelerationBuildGeometryInfo.dstAccelerationStructure = accelerationStructure;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &structureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getAddress();

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = indexCount / 3;
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


VkDeviceAddress BottomAccelerationStructure::getDeviceAddress() const {
	return acDeviceAddress;
}
