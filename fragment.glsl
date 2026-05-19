#version 410 core
out vec4 frag_colour;

in vec3 FragPos;
in vec3 Normal;

uniform vec4 ourColor;       // Базовый цвет объекта
uniform vec3 lightPos;       // Положение источника света
uniform vec3 viewPos;        // Положение камеры (cameraPosition)
uniform vec3 lightColor;     // Цвет света (например, белый vec3(1.0))

void main() {
    // 1. Ambient (Окружающее освещение)
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
    
    // 2. Diffuse (Диффузное освещение)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 3. Specular (Зеркальные блики)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 — это shininess (блеск)
    vec3 specular = specularStrength * spec * lightColor;  
    
    // Итоговый цвет
    vec3 result = (ambient + diffuse + specular) * vec3(ourColor);
    frag_colour = vec4(result, 1.0);
}