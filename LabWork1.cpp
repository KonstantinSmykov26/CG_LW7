#define GLFW_DLL
#define GLEW_DLL
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <cmath>
#include "Shader.h"
#include "Model.h" 

const float PI = 3.14159265359f;

void setMat4(GLuint ID, const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// Позиционирование камеры
glm::vec3 cameraPosition = glm::vec3(0.0f, 2.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* win);

// Переменные для углов и смещений манипулятора
float angleAxis1 = 0.0f;
float angleAxis2 = 0.0f;
float liftY = 0.0f;

// Параметры камеры для обзора мышью
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Sankyo Skilam SR-2 Manipulator", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader myShader("vertex.glsl", "fragment.glsl");
    Model myModel("Model_v2.obj");

    glm::vec3 lightPos(2.0f, 6.0f, 4.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec4 objectColor(0.9f, 0.45f, 0.1f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.use();

        myShader.setVec4("ourColor", objectColor.r, objectColor.g, objectColor.b, objectColor.a);
        glUniform3fv(glGetUniformLocation(myShader.ID, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(myShader.ID, "viewPos"), 1, &cameraPosition[0]);
        glUniform3fv(glGetUniformLocation(myShader.ID, "lightColor"), 1, &lightColor[0]);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        myShader.setMat4("projection", projection);
        myShader.setMat4("view", view);
        myShader.setMat4("model", model);

        // Передаем обновленные переменные
        myModel.Draw(myShader, angleAxis1, angleAxis2, liftY);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* win) {
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    float moveSpeed = 50.0f * deltaTime;

    // Механика изменения углов и высоты по клавишам
    if (glfwGetKey(win, GLFW_KEY_N) == GLFW_PRESS) {
        angleAxis1 -= moveSpeed;
    }
    if (glfwGetKey(win, GLFW_KEY_M) == GLFW_PRESS) {
        angleAxis1 += moveSpeed;
    }

    if (glfwGetKey(win, GLFW_KEY_J) == GLFW_PRESS) {
        angleAxis2 -= moveSpeed;
    }
    if (glfwGetKey(win, GLFW_KEY_K) == GLFW_PRESS) {
        angleAxis2 += moveSpeed;
    }

    if (glfwGetKey(win, GLFW_KEY_U) == GLFW_PRESS) {
        liftY -= moveSpeed * 0.05f;
    }
    if (glfwGetKey(win, GLFW_KEY_I) == GLFW_PRESS) {
        liftY += moveSpeed * 0.05f;
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
