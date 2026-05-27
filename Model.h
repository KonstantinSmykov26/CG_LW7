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
#include "Shader.h"

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

    // Принимаем углы вращения звеньев и линейное смещение по вертикали
    void Draw(Shader& shader, float angleAxis1, float angleAxis2, float liftY)
    {
        // 1. Первая ступень (поворот основания вокруг Y)
        glm::mat4 transform1 = glm::rotate(glm::mat4(1.0f), glm::radians(angleAxis1), glm::vec3(0.0f, 1.0f, 0.0f));

        // 2. Вторая ступень
        // Берём поворот основания и добавляем к нему вращение самого плеча/ротора (по J/K)
        glm::mat4 transform2 = glm::rotate(transform1, glm::radians(angleAxis2), glm::vec3(0.0f, 1.0f, 0.0f));

        // 3. Третья ступень
        // Она полностью наследует положение второй ступени, но двигается вдоль своей локальной оси (например, локальной Y)
        glm::mat4 transform3 = glm::translate(transform2, glm::vec3(0.0f, liftY, 0.0f));


        // Отрисовываем меши
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            switch (i)
            {
            case 1:
                // Если это первое звено (просто поворот основания)
                shader.setMat4("transform", transform1);
                break;

            case 2:
                // ЗЕЛЁНАЯ ЧАСТЬ (наследует baseTransform + крутится по J/K)
                shader.setMat4("transform", transform2);
                break;

            case 4:
                shader.setMat4("transform", transform1);
                break;

            case 5:
                // КРАСНЫЙ ЦИЛИНДР (наследует всё от зелёной части + двигается по U/I)
                shader.setMat4("transform", transform3);
                break;

            default:
                // Для остальных статичных частей (если есть) сбрасываем в identity
                shader.setMat4("transform", glm::mat4(1.0f));
                break;
            }

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

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // ОПЕЧАТКА ИСПРАВЛЕНА: mMhes заменено обратно на mMeshes
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

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

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

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
};

#endif
