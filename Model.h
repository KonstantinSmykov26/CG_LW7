#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

class Model
{
public:
    vector<Mesh> meshes;
    string directory;

    Model(string const& path)
    {
        loadModel(path);
    }

    void Draw()
    {
        // Отрисовываем только если меши действительно загрузились
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw();
        }
    }

private:
    void loadModel(string const& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path,
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        // Защита: если файл не найден или поврежден
        if (nullptr == scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stderr, "ASSIMP ERROR: %s\n", importer.GetErrorString());
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3 vector;

            // Позиции
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // ЗАЩИТА: Проверяем, есть ли нормали в файле, чтобы избежать вылета 0xc0000005
            if (mesh->HasNormals() && mesh->mNormals != nullptr) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            else {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // ИСПРАВЛЕНО: Сбор индексов (без этого шага модель не построится)
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return Mesh(vertices, indices);
    }
};

#endif