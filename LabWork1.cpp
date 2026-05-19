// LabWork1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
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
#include "Model.h" // Подключаем класс для работы с моделями 


// Исправлено: Gluint заменен на GLuint 
void setMat4(GLuint ID, const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// Отодвигаем камеру немного назад по оси Z, чтобы модель попала в кадр
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* win);

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Переменные состояния манипулятора
float angleAxis1 = 0.0f; // Угол поворота первого плеча
float angleAxis2 = 0.0f; // Угол поворота второго плеча
float liftY = 0.0f;      // Вертикальное перемещение штока (ось Y)

float moveSpeed = 0.5f;  // Скорость изменений

// Размеры звеньев робота (значения из чертежа или Blender на замену)
const float Arm1_Length = 1.5f;
const float Arm2_Length = 1.2f;

int main()
{
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: GLFW3 init failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создаем окно (можно увеличить размер до SCR_WIDTH и SCR_HEIGHT для соответствия пропорциям матриц)
    GLFWwindow* myWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello World", NULL, NULL);
    if (!myWindow) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(myWindow);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return 1;

    // Включаем тест глубины, чтобы 3D-модель отображалась корректно (задние грани не накладывались на передние)
    //glEnable(DEPTH_TEST);

    glfwSetFramebufferSizeCallback(myWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(myWindow, mouse_callback);
    glfwSetScrollCallback(myWindow, scroll_callback);
    glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Создание шейдерной программы
    Shader myShader("vertex.glsl", "fragment.glsl");

    // Загрузка 3D-модели. Укажите точный путь к вашему файлу модели (.obj, .fbx и т.д.) 
    Model myModel("Model_v2.obj");

    std::cout << "Mesh count: " << myModel.meshes.size() << std::endl;

    // Основной цикл
    while (!glfwWindowShouldClose(myWindow)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(myWindow);

        // Исправлено: Очищаем не только буфер цвета, но и буфер глубины
        glClearColor(0.4f, 0.6f, 0.9f, 1.0f); // Голубой фон как на скриншоте
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Вычисление матриц
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 transform = glm::mat4(1.0f);

        myShader.use();

        // Задаем цвет объекта (например, красный или ваш текущий)
        myShader.setVec4("ourColor", 0.7f, 0.2f, 0.2f, 1.0f);

        // 1. Передаем позицию камеры для расчета бликов
        glUniform3fv(glGetUniformLocation(myShader.ID, "viewPos"), 1, &cameraPosition[0]);

        // 2. Задаем позицию источника света (можете менять координаты, чтобы свет двигался)
        glm::vec3 lightPos(1.2f, 2.0f, 2.0f);
        glUniform3fv(glGetUniformLocation(myShader.ID, "lightPos"), 1, &lightPos[0]);

        // 3. Задаем цвет источника света (белый)
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(myShader.ID, "lightColor"), 1, &lightColor[0]);

        // ОБЯЗАТЕЛЬНО: Передаем матрицы в uniform-переменные вершинного шейдера 
        setMat4(myShader.ID, "projection", projection);
        setMat4(myShader.ID, "view", view);
        setMat4(myShader.ID, "model", model);
        setMat4(myShader.ID, "transform", transform);

        // Задаем цвет модели (например, оранжевый, как на скриншоте)
        myShader.setVec4("ourColor", 1.0f, 0.5f, 0.0f, 1.0f);

        // Отрисовка загруженной 3D-модели вместо старого vao звезды 
        //myModel.Draw();

        //int testIndex = 0;
        //for (unsigned int i = 0; i < myModel.meshes.size(); i++) {
        //    if (i == testIndex) {
        //        // Красим тестируемый меш в ярко-красный
        //        myShader.setVec4("ourColor", 1.0f, 0.0f, 0.0f, 1.0f);
        //        myModel.meshes[i].Draw();
        //    }
        //    else {
        //        // Остальные меши делаем полупрозрачными/серыми или вообще комментируем строку ниже, чтобы скрыть
        //        myShader.setVec4("ourColor", 0.2f, 0.2f, 0.2f, 0.3f);
        //        myModel.meshes[i].Draw();
        //    }
        //}

        glm::mat4 identityMatrix = glm::mat4(1.0f);
        setMat4(myShader.ID, "transform", identityMatrix);

        // Базовая матрица всей сцены
        glm::mat4 baseModel = glm::mat4(1.0f);

        // 1. ГРУППА: СТАНИНА (Неподвижная стойка)
        myShader.setVec4("ourColor", 0.4f, 0.4f, 0.45f, 1.0f); // Тёмно-серый цвет
        setMat4(myShader.ID, "model", baseModel); // Передали baseModel
        myModel.meshes[0].Draw();


        // 2. ГРУППА: ПЕРВОЕ ПЛЕЧО (Вращение вокруг оси Y)
        glm::mat4 modelAxis1 = baseModel;
        modelAxis1 = glm::rotate(modelAxis1, glm::radians(angleAxis1), glm::vec3(0.0f, 1.0f, 0.0f));

        myShader.setVec4("ourColor", 0.7f, 0.7f, 0.8f, 1.0f); // Стандартный цвет
        setMat4(myShader.ID, "model", modelAxis1); // ИСПРАВЛЕНО: Теперь передаем modelAxis1 в шейдер!
        myModel.meshes[1].Draw(); // Само плечо
        myModel.meshes[3].Draw(); // Крышка сустава


        // 3. ГРУППА: ВТОРОЕ ПЛЕЧО (Наследует поворот плеча 1 + смещается на его длину)
        glm::mat4 modelAxis2 = modelAxis1;
        // Смещаемся вдоль плеча к оси вращения второго сустава
        modelAxis2 = glm::translate(modelAxis2, glm::vec3(Arm1_Length, 0.0f, 0.0f));
        // Вращаем второе плечо вокруг локальной оси Y
        modelAxis2 = glm::rotate(modelAxis2, glm::radians(angleAxis2), glm::vec3(0.0f, 1.0f, 0.0f));

        myShader.setVec4("ourColor", 0.65f, 0.65f, 0.75f, 1.0f);
        setMat4(myShader.ID, "model", modelAxis2); // ИСПРАВЛЕНО: Передаем modelAxis2 в шейдер!
        myModel.meshes[2].Draw(); // Второе плечо
        myShader.setVec4("ourColor", 0.2f, 0.2f, 0.2f, 1.0f); // Чёрный инструмент
        myModel.meshes[4].Draw();


        // 4. ГРУППА: ШТОК И ИНСТРУМЕНТ (Линейный сдвиг вверх/вниз по оси Y)
        glm::mat4 modelTool = modelAxis2;
        // Перемещаемся на конец второго плеча и добавляем liftY для линейного привода
        modelTool = glm::translate(modelTool, glm::vec3(Arm2_Length, liftY, 0.0f));

        setMat4(myShader.ID, "model", modelTool); // ИСПРАВЛЕНО: Передаем modelTool в шейдер!

        myShader.setVec4("ourColor", 0.8f, 0.8f, 0.8f, 1.0f); // Металлический шток
        myModel.meshes[5].Draw();

        

  

        // Проверяем, что меши робота загружены корректно
        //if (myModel.meshes.size() >= 4)
        //{
        //    // Базовая матрица всей сцены (основание робота)
        //    glm::mat4 baseModel = glm::mat4(1.0f);

        //    // --- Степень 0: Вертикальная стойка (Неподвижный базовый элемент) ---
        //    setMat4(myShader.ID, "model", baseModel);
        //    myModel.meshes[0].Draw();

        //    // --- Степень 1: Поворот первого модуля в горизонтальной плоскости (01-Axis) ---
        //    glm::mat4 modelAxis1 = baseModel;
        //    // Вращаем вокруг вертикальной оси Y
        //    modelAxis1 = glm::rotate(modelAxis1, glm::radians(angleAxis1), glm::vec3(0.0f, 1.0f, 0.0f));

        //    setMat4(myShader.ID, "model", modelAxis1);
        //    myModel.meshes[1].Draw();

        //    // --- Степень 2: Поворот второго модуля в горизонтальной плоскости (02-Axis) ---
        //    // Наследует все трансформации плеча 1 (modelAxis1)
        //    glm::mat4 modelAxis2 = modelAxis1;
        //    // Смещаемся на конец первого плеча к локальной оси вращения второго сустава
        //    modelAxis2 = glm::translate(modelAxis2, glm::vec3(Arm1_Length, 0.0f, 0.0f));
        //    // Вращаем вокруг локальной оси Y второго сустава
        //    modelAxis2 = glm::rotate(modelAxis2, glm::radians(angleAxis2), glm::vec3(0.0f, 1.0f, 0.0f));

        //    setMat4(myShader.ID, "model", modelAxis2);
        //    myModel.meshes[2].Draw();

        //    // --- Степень 3: Вертикальное линейное перемещение основной части (Шток / Схват) ---
        //    // Наследует вращения обоих плеч (modelAxis2)
        //    glm::mat4 modelTool = modelAxis2;
        //    // Смещаемся на рабочую длину второго плеча и сдвигаем по Y на liftY (вверх/вниз)
        //    modelTool = glm::translate(modelTool, glm::vec3(Arm2_Length, liftY, 0.0f));

        //    setMat4(myShader.ID, "model", modelTool);
        //    myModel.meshes[3].Draw();
        //}
        //else
        //{
        //    // На случай, если модель загружена одним целым куском без разделения на меши
        //    glm::mat4 identityModel = glm::mat4(1.0f);
        //    setMat4(myShader.ID, "model", identityModel);
        //    myModel.Draw();
        //}

        glfwSwapBuffers(myWindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* win) {
    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPosition += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPosition -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
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
        liftY -= moveSpeed * 0.01f;
    }
    if (glfwGetKey(win, GLFW_KEY_I) == GLFW_PRESS) {
        liftY += moveSpeed * 0.01f; // небольшие шаги для перемещения
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
    float yoffset = lastY - ypos; // Исправлено: инверсия вертикальной оси мыши

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}