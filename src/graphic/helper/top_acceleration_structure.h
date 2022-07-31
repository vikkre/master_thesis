#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer.h"
#include "../../init_exception.h"


class Device;

class TopAccelerationStructure {
	public:
		TopAccelerationStructure(const Device* device);
		~TopAccelerationStructure();

		void init();

		const Buffer& getBuffer() const;
		const VkAccelerationStructureKHR& getAccelerationStructure() const;
		const VkDeviceAddress& getAddress() const;

		std::vector<VkAccelerationStructureInstanceKHR> blasInstances;
	private:
		void createInstancesBuffer();
		void getBuildSize();
		void createScratchBuffer();
		void createAccelerationStructureBuffer();
		void buildAccelerationStructure();

		const Device* device;

		VkAccelerationStructureGeometryKHR structureGeometry;
		VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo;
		VkAccelerationStructureKHR accelerationStructure;

		Buffer instancesBuffer;
		Buffer acBuffer;
		VkDeviceAddress acDeviceAddress;
		Buffer scratchBuffer;
};
