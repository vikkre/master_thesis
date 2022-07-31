#include "mesh.h"

#include "device.h"


#define BUFFER_USAGE VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR


Mesh::Mesh(const Device* device)
:vertices(), indices(),
device(device),
vertexBuffer(device), indexBuffer(device), blas(device) {}

Mesh::~Mesh() {
	vkDeviceWaitIdle(device->getDevice());
	
	vertexBuffers.clear();
	vertexBufferOffsets.clear();
}

void Mesh::init() {
	vkDeviceWaitIdle(device->getDevice());

	createVertexBuffer();
	createIndexBuffer();
	createBlas();
}

void Mesh::recordCommandBuffer(const VkCommandBuffer* commandBuffer) const {
	vkCmdBindVertexBuffers(*commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
	vkCmdBindIndexBuffer(*commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(*commandBuffer, indices.size(), 1, 0, 0, 0);
}

void Mesh::addPoint(const Vector3f& point, const Vector3f& normal) {
	Mesh::Vertex vertex{};

	vertex.point = point;
	vertex.normal = normal;

	vertices.push_back(vertex);
}

void Mesh::addIndex(const Vector3u& index) {
	indices.push_back(index[0]);
	indices.push_back(index[1]);
	indices.push_back(index[2]);
}

void Mesh::createVertexBuffer() {
	vertexBuffer.bufferSize = sizeof(vertices[0]) * vertices.size();
	vertexBuffer.usage = BUFFER_USAGE | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBuffer.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vertexBuffer.init();
	vertexBuffer.passData(vertices.data());

	vertexBuffers.push_back(vertexBuffer.getBuffer());
	vertexBufferOffsets.push_back(0);
}

void Mesh::createIndexBuffer() {
	indexBuffer.bufferSize = sizeof(indices[0]) * indices.size();
	indexBuffer.usage = BUFFER_USAGE | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBuffer.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	indexBuffer.init();
	indexBuffer.passData(indices.data());
}

void Mesh::createBlas() {
	blas.indexCount        = indices.size();
	blas.vertexCount       = vertices.size();
	blas.vertexPointOffset = offsetof(Mesh::Vertex, point);
	blas.vertexStride      = sizeof(Mesh::Vertex);
	
	blas.indexAddress  = indexBuffer.getAddress();
	blas.vertexAddress = vertexBuffer.getAddress();

	blas.init();
}

const DataBuffer& Mesh::getVertexBuffer() const {
	return vertexBuffer;
}

const DataBuffer& Mesh::getIndexBuffer() const {
	return indexBuffer;
}

const BottomAccelerationStructure& Mesh::getBlas() const {
	return blas;
}

VkVertexInputBindingDescription Mesh::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Mesh::Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Mesh::getAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkVertexInputAttributeDescription positionAttribute{};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Mesh::Vertex, point);
	attributeDescriptions.push_back(positionAttribute);

	VkVertexInputAttributeDescription normalAttribute{};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(Mesh::Vertex, normal);
	attributeDescriptions.push_back(normalAttribute);

	return attributeDescriptions;
}
