#include "Model.h"
#include "LightMapperDefine.h"

#include <glm.hpp>

void CombindVertexData(void* dst, void* src, uint32_t vertexCount, uint32_t attributeSize, uint32_t offset, uint32_t stride, uint32_t size) {
    uint8_t* dstData = (uint8_t*)dst + offset;
    uint8_t* srcData = (uint8_t*)src;
    for (uint32_t i = 0; i < vertexCount; i++) {
        memcpy(dstData, srcData, attributeSize);
        dstData += stride;
        srcData += attributeSize;
    }
}

Model::Model() {
}

Model::~Model() {
    SAFE_DELETE_ARRAY(position_data);
    SAFE_DELETE_ARRAY(normal_data);
    SAFE_DELETE_ARRAY(uv0_data);
    SAFE_DELETE_ARRAY(uv1_data);
    SAFE_DELETE_ARRAY(index_data);
}

void Model::ResetPositionData(uint8_t* position_data) {
    SAFE_DELETE_ARRAY(this->position_data);
    this->position_data = position_data;
}

void Model::ResetNormalData(uint8_t* normal_data) {
    SAFE_DELETE_ARRAY(this->normal_data);
    this->normal_data = normal_data;
}

void Model::ResetUV0Data(uint8_t* uv0_data) {
    SAFE_DELETE_ARRAY(this->uv0_data);
    this->uv0_data = uv0_data;
}

void Model::ResetUV1Data(uint8_t* uv1_data) {
    SAFE_DELETE_ARRAY(this->uv1_data);
    this->uv1_data = uv1_data;
}

void Model::ResetIndexData(uint8_t* index_data) {
    SAFE_DELETE_ARRAY(this->index_data);
    this->index_data = index_data;
}

void Model::GenerateGPUResource(blast::GfxDevice* device) {
    uint8_t* vertex_data = new uint8_t[vertex_count * sizeof(MeshVertex)];
    CombindVertexData(vertex_data, position_data, vertex_count, sizeof(glm::vec3), offsetof(MeshVertex, position), sizeof(MeshVertex), sizeof(glm::vec3) * vertex_count);
    CombindVertexData(vertex_data, normal_data, vertex_count, sizeof(glm::vec3), offsetof(MeshVertex, normal), sizeof(MeshVertex), sizeof(glm::vec3) * vertex_count);
    CombindVertexData(vertex_data, uv0_data, vertex_count, sizeof(glm::vec2), offsetof(MeshVertex, uv0), sizeof(MeshVertex), sizeof(glm::vec2) * vertex_count);
    CombindVertexData(vertex_data, uv1_data, vertex_count, sizeof(glm::vec2), offsetof(MeshVertex, uv1), sizeof(MeshVertex), sizeof(glm::vec2) * vertex_count);

    blast::GfxCommandBuffer* copy_cmd = device->RequestCommandBuffer(blast::QUEUE_COPY);
    blast::GfxBufferDesc buffer_desc;
    buffer_desc.size = sizeof(MeshVertex) * vertex_count;
    buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
    buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER | blast::RESOURCE_USAGE_RW_BUFFER;
    vertex_buffer = device->CreateBuffer(buffer_desc);
    {
        device->UpdateBuffer(copy_cmd, vertex_buffer, vertex_data, sizeof(MeshVertex) * vertex_count);
        blast::GfxBufferBarrier barrier;
        barrier.buffer = vertex_buffer;
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE | blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(copy_cmd, 1, &barrier, 0, nullptr);
    }

    if (index_type == blast::INDEX_TYPE_UINT16) {
        buffer_desc.size = sizeof(uint16_t) * index_count;
    } else {
        buffer_desc.size = sizeof(uint32_t) * index_count;
    }
    buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
    buffer_desc.res_usage = blast::RESOURCE_USAGE_INDEX_BUFFER;
    index_buffer = device->CreateBuffer(buffer_desc);
    {
        device->UpdateBuffer(copy_cmd, index_buffer, index_data, buffer_desc.size);
        blast::GfxBufferBarrier barrier;
        barrier.buffer = index_buffer;
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE | blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(copy_cmd, 1, &barrier, 0, nullptr);
    }

    SAFE_DELETE_ARRAY(vertex_data);
}

void Model::ReleaseGPUResource(blast::GfxDevice* device) {
    device->DestroyBuffer(vertex_buffer);
    device->DestroyBuffer(index_buffer);
}