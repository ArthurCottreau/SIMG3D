//
// Created by Bellaedris on 12/12/2025.
//

#include <iostream>
#include <set>
#include "Mesh.h"

#include "GPU/Texture.h"
#include <memory>
#include <cstring>

namespace mgl
{
    std::vector<std::unique_ptr<mgl::gpu::Texture>> t;
    
    Mesh::Mesh(const char *filename)
    {
        tinyobj::ObjReaderConfig reader_config;

        tinyobj::ObjReader reader;

        if (reader.ParseFromFile(filename, reader_config) == false)
        {
            if (!reader.Error().empty())
            {
                std::cerr << "TinyObjReader Error: " << reader.Error();
            }
            return;
        }

        if (reader.Warning().empty() == false)
        {
            std::cout << "TinyObjReader Warning: " << reader.Warning();
        }
        
        const tinyobj::attrib_t& attrib = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
        const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();
        std::map<VertexKey, uint32_t> uniqueVertices;
        int got_mat = 0;

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++)
        {
            shape_data.push_back({std::vector<VertexData>(),std::vector<uint32_t>(),Texture()});

            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t face = 0; face < shapes[s].mesh.num_face_vertices.size(); face++)
            {
                size_t faceVertices = size_t(shapes[s].mesh.num_face_vertices[face]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < faceVertices; v++)
                {
                    VertexData vertex;
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    VertexKey key{ idx.vertex_index, idx.texcoord_index, idx.normal_index };

                    if(uniqueVertices.find(key) == uniqueVertices.end())
                    {
                        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                        vertex.pos = {vx, vy, vz};

                        // Check if `normal_index` is zero or positive. negative = no normal data
                        if (idx.normal_index >= 0)
                        {
                            tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                            tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                            tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                            vertex.normal = {nx, ny, nz};
                        }

                        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                        if (idx.texcoord_index >= 0)
                        {
                            tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                            tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                            vertex.texcoord = {tx, ty};
                        }
                        
                        uniqueVertices[key] = shape_data[s].m_verticesData.size();
                        shape_data[s].m_verticesData.push_back(vertex);
                    }
                    shape_data[s].m_indices.push_back(uniqueVertices[key]);
                }
                index_offset += faceVertices;
                
                // per-face material
                int id = shapes[s].mesh.material_ids[face];
                if (got_mat == 0)
                {
                    Texture text {id, materials[id]};
                    shape_data[s].texture_data = text;

                    got_mat = 1;
                }
            }
            got_mat = 0;
        }

        // fill in GPU datas
        SetupGPU();
    }
    
    Mesh::Mesh(std::vector<ShapeData>& shapes)
        : shape_data(std::move(shapes))
    {
        SetupGPU();
    }

    void Mesh::Draw() const
    {
        for (size_t s = 0; s < shape_data.size(); s++)
        {
            t[s]->Bind(0);

            vao_data[s].Bind();
            glDrawElements(GL_TRIANGLES, shape_data[s].m_indices.size(), gpu::GLUtils::GetDataType(gpu::GLUtils::DataType::UnsignedInt), nullptr);
            vao_data[s].Unbind();
        }
    }

    void Mesh::SetupGPU()
    {
        std::string path = "resources/models/";
        
        for (size_t s = 0; s < shape_data.size(); s++)
        {
            vao_data.push_back({gpu::Vao ()});
            buffer_data.push_back({gpu::Buffer::BufferType::Vertex, gpu::Buffer::BufferType::Index});

            std::string text_file = shape_data[s].texture_data.tex.diffuse_texname;
            std::string full_path = path + text_file;

            t.push_back(std::make_unique<mgl::gpu::Texture>(mgl::gpu::Texture::TextureTarget::Target2D, full_path.c_str(), true));
            
            vao_data[s].Bind();
            buffer_data[s].m_buffer.Bind();
            buffer_data[s].m_buffer.Write(sizeof(VertexData) * shape_data[s].m_verticesData.size(), shape_data[s].m_verticesData.data(), gpu::Buffer::BufferUsage::StaticDraw);
            
            buffer_data[s].m_indexBuffer.Bind();
            buffer_data[s].m_indexBuffer.Write(sizeof(uint32_t) * shape_data[s].m_indices.size(), shape_data[s].m_indices.data(), gpu::Buffer::BufferUsage::StaticDraw);

            vao_data[s].SetAttribute(0, gpu::GLUtils::DataType::Float, 0, 3, sizeof(VertexData)); // positions
            vao_data[s].SetAttribute(1, gpu::GLUtils::DataType::Float, offsetof(VertexData, normal), 3, sizeof(VertexData)); // normals
            vao_data[s].SetAttribute(2, gpu::GLUtils::DataType::Float, offsetof(VertexData, texcoord), 2, sizeof(VertexData)); // texcoords

            vao_data[s].Unbind();
        }
    }
} // mgl