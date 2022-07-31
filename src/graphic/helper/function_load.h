#pragma once

#include <vulkan/vulkan.h>


class FuncLoad {
	private:
		static VkDevice device;

		template<typename T>
		static T getFunction(const char* name) {
			return reinterpret_cast<T>(vkGetDeviceProcAddr(device, name));
		}

	public:
		static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		static PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
		static PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
		static PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		static PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;

		static void init(VkDevice device) {
			FuncLoad::device = device;

			FuncLoad::vkGetAccelerationStructureBuildSizesKHR =
				FuncLoad::getFunction<PFN_vkGetAccelerationStructureBuildSizesKHR>
				("vkGetAccelerationStructureBuildSizesKHR");

			FuncLoad::vkCreateAccelerationStructureKHR =
				FuncLoad::getFunction<PFN_vkCreateAccelerationStructureKHR>
				("vkCreateAccelerationStructureKHR");

			FuncLoad::vkGetAccelerationStructureDeviceAddressKHR =
				FuncLoad::getFunction<PFN_vkGetAccelerationStructureDeviceAddressKHR>
				("vkGetAccelerationStructureDeviceAddressKHR");

			FuncLoad::vkDestroyAccelerationStructureKHR =
				FuncLoad::getFunction<PFN_vkDestroyAccelerationStructureKHR>
				("vkDestroyAccelerationStructureKHR");

			FuncLoad::vkCmdBuildAccelerationStructuresKHR =
				FuncLoad::getFunction<PFN_vkCmdBuildAccelerationStructuresKHR>
				("vkCmdBuildAccelerationStructuresKHR");

			FuncLoad::vkGetBufferDeviceAddressKHR =
				FuncLoad::getFunction<PFN_vkGetBufferDeviceAddressKHR>
				("vkGetBufferDeviceAddressKHR");

			FuncLoad::vkCreateRayTracingPipelinesKHR =
				FuncLoad::getFunction<PFN_vkCreateRayTracingPipelinesKHR>
				("vkCreateRayTracingPipelinesKHR");

			FuncLoad::vkGetRayTracingShaderGroupHandlesKHR =
				FuncLoad::getFunction<PFN_vkGetRayTracingShaderGroupHandlesKHR>
				("vkGetRayTracingShaderGroupHandlesKHR");

			FuncLoad::vkCmdTraceRaysKHR =
				FuncLoad::getFunction<PFN_vkCmdTraceRaysKHR>
				("vkCmdTraceRaysKHR");
		}
};
