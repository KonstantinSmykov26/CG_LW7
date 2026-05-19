#version 410 core
layout (location = 0) in vec3 vp;     // Позиция из Mesh.h
layout (location = 1) in vec3 normal; // Нормаль из Mesh.h

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model; // Оставляем только model
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Чистый расчет без transform
    FragPos = vec3(model * vec4(vp, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}