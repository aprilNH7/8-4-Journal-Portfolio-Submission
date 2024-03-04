#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/SOIL2.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Vertex shader source
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec3 aNormal;

    out vec2 TexCoord;
    out vec3 FragPos;
    out vec3 Normal;
    out vec3 ViewDir;
    out vec3 halfwayVec;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        TexCoord = aTexCoord;
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        ViewDir = vec3(inverse(view) * vec4(0.0, 0.0, 0.0, 1.0) - vec4(FragPos, 1.0));
        vec3 lightDir = vec3(1.0, 1.0, 1.0); // directional light direction
        halfwayVec = normalize(lightDir + ViewDir);
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

// Fragment shader source with brightness adjustment
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    in vec3 FragPos;
    in vec3 Normal;
    in vec3 ViewDir;
    in vec3 halfwayVec;

    out vec4 FragColor;

    uniform sampler2D ourTexture;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    uniform vec3 lightDir;
    uniform float shininess;
    uniform float brightness; // New uniform for brightness adjustment

    void main()
    {
        // Ambient light
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse light
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular light
        float spec = pow(max(dot(Normal, halfwayVec), 0.0), shininess);
        vec3 specular = spec * lightColor;

        // Calculate final color with brightness adjustment
        vec3 result = (ambient + diffuse + specular) * texture(ourTexture, TexCoord).rgb * brightness;

        FragColor = vec4(result, 1.0);
    }
)";


int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Shadowy Effect with Texture Mapping", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Define the vertices, texture coordinates, and normals of a cube
    float vertices[] = {
        // Positions          // Texture Coords  // Normals
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,        0.0f, 0.0f, 1.0f, // Front face
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,        0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,        0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,        0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,        0.0f, 0.0f, -1.0f, // Back face
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,        0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,        0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,        0.0f, 0.0f, -1.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,        -1.0f, 0.0f, 0.0f, // Left face
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,        -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f,        -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,        -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.0f, 0.0f,        1.0f, 0.0f, 0.0f, // Right face
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,        1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,        1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f,        1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,        0.0f, -1.0f, 0.0f, // Bottom face
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,        0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,        0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,        0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,        0.0f, 1.0f, 0.0f, // Top face
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,        0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,        0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,        0.0f, 1.0f, 0.0f
    };

    // Define the indices to draw the cube
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };
 

    // Create vertex buffer object, vertex array object, and element buffer object
    unsigned int VBO, VAO, EBO, texCoordVBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    // Bind the vertex array object
    glBindVertexArray(VAO);

    // Bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind the VAO
    glBindVertexArray(0);

    // Create and compile the vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create and compile the fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader program link errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load and create a texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char* data = SOIL_load_image("abc.JPG", &width, &height, &nrChannels, SOIL_LOAD_RGB);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    SOIL_free_image_data(data);

    // Set directional light direction
    glm::vec3 lightDir(-1.0f, -1.0f, -1.0f);

    // Set up object and light colors
    glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    // Set shininess (specular intensity)
    float shininess = 64.0f; // Increased shininess for more specular highlights

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set up view and projection matrices for perspective projection
        glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(30.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Set the light properties
        unsigned int lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));
        unsigned int shininessLoc = glGetUniformLocation(shaderProgram, "shininess");
        glUniform1f(shininessLoc, shininess);

        // Set brightness uniform in the fragment shader
        float brightness = 1.2f; // Increase brightness by 20%
        int brightnessLoc = glGetUniformLocation(shaderProgram, "brightness");
        glUniform1f(brightnessLoc, brightness);

        // Rotate the cube uniformly
        float angle = (float)glfwGetTime() * 50.0f; // Rotate 50 degrees per second
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));



        // Bind texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // Draw the cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
