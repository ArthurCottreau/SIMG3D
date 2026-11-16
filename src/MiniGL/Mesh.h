//
// Created by Bellaedris on 12/12/2025.
//

#pragma once

#include <TinyObj/tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include "GPU/Buffer.h"
#include "GPU/VAO.h"

#include "VectorUtils.h"

namespace mgl
{
class Mesh
{
public:
    #pragma region structs
    /**
     * Our meshes store 3 data: positions, normals and texture coordinates
     */
    struct VertexData
    {
        glm::vec3 pos, normal {};
        glm::vec2 texcoord {};
    };

    struct Texture {
        unsigned int id;
        tinyobj::material_t tex;
    };

    struct ShapeData {
        std::vector<VertexData> m_verticesData;
        std::vector<uint32_t> m_indices;
        Texture texture_data;
    };

    struct BufferData
    {
        gpu::Buffer m_buffer, m_indexBuffer;
    };

    struct VertexKey {
        int vertexIndex;
        int texcoordIndex;
        int normalIndex;

        bool operator<(const VertexKey& other) const {
            if (vertexIndex  != other.vertexIndex)
                return vertexIndex  < other.vertexIndex;

            if (texcoordIndex != other.texcoordIndex)
                return texcoordIndex < other.texcoordIndex;

            return normalIndex < other.normalIndex;
        }
    };
    #pragma endregion structs

private:
    #pragma region Members
    /* TODO Right now, we delete the vertices after they are sent to the GPU since we don't want duplicated datas.
     * Some people might prefer to keep them CPU-side as well and recall SetupGPU() afterwards, others may want to have both by
     * reading vertexData from GPU, write these data into separate per-component vectors, then rebuild a vertexData array when
     these data change, kind of what unity API has.
     */

    /**
     * We store an array of vertex data, and an array of indices, since most 3D objects come in indexed form
     */
    std::vector<ShapeData> shape_data;

    /** \brief Buffers that will store vertices and indices data */
    std::vector<BufferData> buffer_data;
    std::vector<gpu::Vao> vao_data;
    #pragma endregion Members
public:
    /**
     * Construct a mesh from an obj file.
     * @param filename path to the obj file
     */
    Mesh(const char* filename);

    /**
     * Construct a mesh from an array of vertices and indices. Allows to create meshes by hand, without reading an obj
     * @param vertices array of vertices
     * @param indices array of indices
     */
    Mesh(std::vector<ShapeData>& shape_data);

    /**
     * Sets up our buffer datas, their VAO and sends them to the GPU
     */
    void SetupGPU();

    void Draw() const;
};
} // mgl