#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer.h"
#include "data_buffer.h"
#include "../../init_exception.h"


class Device;

class TopAccelerationStructureBuffer: public Buffer {
	public:
		TopAccelerationStructureBuffer(const Device* device);
		~TopAccelerationStructureBuffer();

		void init();
		void updateUniforms();
		void cmdUpdate(VkCommandBuffer commandBuffer);

		const DataBuffer& getBuffer() const;
		const VkAccelerationStructureKHR& getAccelerationStructure() const;
		const VkDeviceAddress& getAddress() const;
		virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
		virtual VkDescriptorType getDescriptorType() const override;

		struct Properties {
			std::vector<VkAccelerationStructureInstanceKHR> blasInstances;
		} properties;
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

		DataBuffer instancesBuffer;
		DataBuffer acBuffer;
		VkDeviceAddress acDeviceAddress;
		DataBuffer scratchBuffer;
		VkWriteDescriptorSetAccelerationStructureKHR tlasWriteSetStructure;
};
