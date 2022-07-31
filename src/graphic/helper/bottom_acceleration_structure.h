#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer.h"
#include "../../init_exception.h"


class Device;

class BottomAccelerationStructure {
	public:
		BottomAccelerationStructure(const Device* device);
		~BottomAccelerationStructure();

		void init();

		VkDeviceAddress getDeviceAddress() const;

		uint32_t indexCount;
		uint32_t vertexCount;
		uint32_t vertexPointOffset;
		VkDeviceSize vertexStride;

		VkDeviceAddress indexAddress;
		VkDeviceAddress vertexAddress;
	private:
		void createStructureGeometry();
		void getBuildSize();
		void createAccelerationStructure();
		void createScratchBuffer();
		void computeAccelerationStructure();

		const Device* device;

		VkAccelerationStructureGeometryKHR structureGeometry;
		VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo;
		VkAccelerationStructureKHR accelerationStructure;

		Buffer acBuffer;
		VkDeviceAddress acDeviceAddress;
		Buffer scratchBuffer;
};
