#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>

using namespace std;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
    Face(int v[], int n[]) {
        vIndex[0] = v[0];
        vIndex[1] = v[1];
        vIndex[2] = v[2];
        nIndex[0] = n[0];
        nIndex[1] = n[1];
        nIndex[2] = n[2];
    }
    GLuint vIndex[3], nIndex[3];
};

vector<float> tpVertices;
vector<unsigned int> tpIndices;
vector<float> aVertices;
vector<unsigned int> aIndices;

unsigned int cubemap;
unsigned int framebuffer;
unsigned int depthbuffer;

float proectionAngle = 45.0f;
float aspectRatio = 1024.0f/768.0f;
float near = 0.1f;
float far = 1000.0f;
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 30.0f);
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 30.0f);

bool firstMouse = true;
float yaw = -90.f;
float pitch = 0.f;
float lastX = 1024.0f / 2.0f;
float lastY = 768.0f / 2.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float orbitDegree = 0.0f;
float teapotAngle = 0.0f;

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

vector<string> skyboxFaceLocations = {
    "res\\textures\\right.jpg",
    "res\\textures\\left.jpg",
    "res\\textures\\top.jpg",
    "res\\textures\\bottom.jpg",
    "res\\textures\\front.jpg",
    "res\\textures\\back.jpg"
};

bool ParseObj(const string& fileName, vector<float>& ver, vector<unsigned int>& ind)
{
    vector<Vertex> vertices;
    vector<Normal> normals;
    vector<Face> faces;

    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == 'v')
                {
                    if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        normals.push_back(Normal(c1, c2, c3));
                    }
                    else if (curLine[1] == 't')
                    {
                        str >> tmp;
                        str >> c1 >> c2;
                        continue;
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        vertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
                    char c;
                    int vIndex[3], nIndex[3];
                    str >> vIndex[0]; str >> c >> c; // consume "//"
                    str >> nIndex[0];
                    str >> vIndex[1]; str >> c >> c; // consume "//"
                    str >> nIndex[1];
                    str >> vIndex[2]; str >> c >> c; // consume "//"
                    str >> nIndex[2];

                    assert(vIndex[0] == nIndex[0] &&
                        vIndex[1] == nIndex[1] &&
                        vIndex[2] == nIndex[2]); // a limitation for now

                 // make indices start from 0
                    for (int c = 0; c < 3; ++c)
                    {
                        vIndex[c] -= 1;
                        nIndex[c] -= 1;
                    }

                    faces.push_back(Face(vIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    assert(vertices.size() == normals.size());

    for (int i = 0; i < vertices.size(); i++)
    {
        ver.push_back(vertices[i].x);
        ver.push_back(vertices[i].y);
        ver.push_back(vertices[i].z);
        ver.push_back(normals[i].x);
        ver.push_back(normals[i].y);
        ver.push_back(normals[i].z);
    }
    for (int i = 0; i < faces.size(); i++)
    {
        ind.push_back(faces[i].vIndex[0]);
        ind.push_back(faces[i].vIndex[1]);
        ind.push_back(faces[i].vIndex[2]);
    }

    return true;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
    {
        firstMouse = true;
        return;
    }
    if (firstMouse)
    {
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

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection = glm::normalize(direction);
}

void HandleKeyPress(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const float cameraSpeed = 4.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPosition += cameraSpeed * cameraDirection;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPosition -= cameraSpeed * cameraDirection;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosition += cameraSpeed * glm::normalize(glm::cross(cameraDirection, cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPosition -= cameraSpeed * glm::normalize(glm::cross(cameraDirection, cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cameraPosition -= cameraSpeed * cameraUp;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cameraPosition += cameraSpeed * cameraUp;
    }
}

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else 
        {
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << 
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

unsigned int loadCubemap(vector<string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void initDynamicCubemap()
{
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 256, 256);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemap, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

unsigned int createDynamicCubemap(unsigned int skyboxShader, unsigned int cubemapVAO, unsigned int cubemapTexture, 
    unsigned int basicShader, unsigned int armadilloVAO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glViewport(0, 0, 256, 256);

    for (int face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemap, 0);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 1000.0f);
        glm::mat4 viewMatrix;
        switch (face)
        {
            case 0:
                viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case 1:
                viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case 2:
                viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case 3:
                viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
                break;
            case 4:
                viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
            case 5:
                viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                break;
        }

        // ---------------------- Draw Skybox -------------------------
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glUseProgram(skyboxShader);
        glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));
        GLint projectionMatrixId = glGetUniformLocation(skyboxShader, "ProjectionMatrix");
        glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
        GLint viewMatrixId = glGetUniformLocation(skyboxShader, "ViewMatrix");
        glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &skyboxViewMatrix[0][0]);
        glBindVertexArray(cubemapVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        // ---------------------- Draw Armadillo -------------------------
        glUseProgram(basicShader);
        GLint degreeId = glGetUniformLocation(basicShader, "orbitDegree");
        glUniform1f(degreeId, orbitDegree);
        GLint cameraPositionId = glGetUniformLocation(basicShader, "CameraPosition");
        glUniform3fv(cameraPositionId, 1, &cameraPosition[0]);
        GLint lightPositionId = glGetUniformLocation(basicShader, "LightPosition");
        glUniform3fv(lightPositionId, 1, &lightPosition[0]);
        glm::mat4 modelMatrix2 = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));
        GLint modelMatrixId2 = glGetUniformLocation(basicShader, "ModelMatrix");
        glUniformMatrix4fv(modelMatrixId2, 1, GL_FALSE, &modelMatrix2[0][0]);
        GLint projectionMatrixId3 = glGetUniformLocation(basicShader, "ProjectionMatrix");
        glUniformMatrix4fv(projectionMatrixId3, 1, GL_FALSE, &projectionMatrix[0][0]);
        GLint viewMatrixId3 = glGetUniformLocation(basicShader, "ViewMatrix");
        glUniformMatrix4fv(viewMatrixId3, 1, GL_FALSE, &viewMatrix[0][0]);
        glBindVertexArray(armadilloVAO);
        glDrawElements(GL_TRIANGLES, aIndices.size(), GL_UNSIGNED_INT, nullptr);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glViewport(0, 0, 1024, 768);

    return cubemap;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 768, "CubeMap", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK)
        std::cout << "Glew is not initialized!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    unsigned int cubemapVAO;
    glGenVertexArrays(1, &cubemapVAO);
    glBindVertexArray(cubemapVAO);
    unsigned int cubemapVBO;
    glGenBuffers(1, &cubemapVBO);
    unsigned int cubemapIBO;
    glGenBuffers(1, &cubemapIBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * 3 * sizeof(float), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glEnable(GL_DEPTH_TEST);

    ParseObj("res/objects/teapot.obj", tpVertices, tpIndices);

    unsigned int teapotVAO;
    glGenVertexArrays(1, &teapotVAO);
    glBindVertexArray(teapotVAO);
    unsigned int teapotVBO;
    glGenBuffers(1, &teapotVBO);
    unsigned int teapotIBO;
    glGenBuffers(1, &teapotIBO);

    glBindBuffer(GL_ARRAY_BUFFER, teapotVBO);
    glBufferData(GL_ARRAY_BUFFER, tpVertices.size() * sizeof(float), &tpVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, teapotIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tpIndices.size() * sizeof(unsigned int), &tpIndices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));

    ParseObj("res/objects/armadillo.obj", aVertices, aIndices);

    unsigned int armadilloVAO;
    glGenVertexArrays(1, &armadilloVAO);
    glBindVertexArray(armadilloVAO);
    unsigned int armadilloVBO;
    glGenBuffers(1, &armadilloVBO);
    unsigned int armadilloIBO;
    glGenBuffers(1, &armadilloIBO);

    glBindBuffer(GL_ARRAY_BUFFER, armadilloVBO);
    glBufferData(GL_ARRAY_BUFFER, aVertices.size() * sizeof(float), &aVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, armadilloIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, aIndices.size() * sizeof(unsigned int), &aIndices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));

    unsigned int cubemapTexture = loadCubemap(skyboxFaceLocations);

    ShaderProgramSource skyboxSource = ParseShader("res/shaders/Skybox.shader");
    unsigned int skyboxShader = CreateShader(skyboxSource.VertexSource, skyboxSource.FragmentSource);

    ShaderProgramSource teapotSource = ParseShader("res/shaders/Mirror.shader");
    unsigned int teapotShader = CreateShader(teapotSource.VertexSource, teapotSource.FragmentSource);
    
    ShaderProgramSource basicSource = ParseShader("res/shaders/Basic.shader");
    unsigned int basicShader = CreateShader(basicSource.VertexSource, basicSource.FragmentSource);

    initDynamicCubemap();

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0)
        {
            string title = "FPS: " + to_string(nbFrames);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        /* Render here */
        glClearColor(0.50, 0.81, 0.92, 1); // Background color
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        HandleKeyPress(window);

        // ---------------------- Draw Skybox -------------------------
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glUseProgram(skyboxShader);
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(proectionAngle), aspectRatio, near, far);
        glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
        glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));
        GLint projectionMatrixId = glGetUniformLocation(skyboxShader, "ProjectionMatrix");
        glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
        GLint viewMatrixId = glGetUniformLocation(skyboxShader, "ViewMatrix");
        glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &skyboxViewMatrix[0][0]);
        glBindVertexArray(cubemapVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        unsigned int environmentMap = createDynamicCubemap(skyboxShader, cubemapVAO, cubemapTexture, basicShader, armadilloVAO);

        // ---------------------- Draw Teapot -------------------------
        glUseProgram(teapotShader);
        teapotAngle += 0.010f;
        GLint angleId = glGetUniformLocation(teapotShader, "Angle");
        glUniform1f(angleId, teapotAngle);
        glm::mat4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, -1.0f, 0.0f));
        GLint modelMatrixId = glGetUniformLocation(teapotShader, "ModelMatrix");
        glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);
        GLint projectionMatrixId2 = glGetUniformLocation(teapotShader, "ProjectionMatrix");
        glUniformMatrix4fv(projectionMatrixId2, 1, GL_FALSE, &projectionMatrix[0][0]);
        GLint viewMatrixId2 = glGetUniformLocation(teapotShader, "ViewMatrix");
        glUniformMatrix4fv(viewMatrixId2, 1, GL_FALSE, &viewMatrix[0][0]);
        GLint cameraPosId = glGetUniformLocation(teapotShader, "cameraPos");
        glUniform3fv(cameraPosId, 1, &cameraPosition[0]);
        glBindVertexArray(teapotVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
        glDrawElements(GL_TRIANGLES, tpIndices.size(), GL_UNSIGNED_INT, nullptr);


        // ---------------------- Draw Armadillo -------------------------
        glUseProgram(basicShader);
        orbitDegree += 0.015;
        GLint degreeId = glGetUniformLocation(basicShader, "orbitDegree");
        glUniform1f(degreeId, orbitDegree);
        GLint cameraPositionId = glGetUniformLocation(basicShader, "CameraPosition");
        glUniform3fv(cameraPositionId, 1, &cameraPosition[0]);
        GLint lightPositionId = glGetUniformLocation(basicShader, "LightPosition");
        glUniform3fv(lightPositionId, 1, &lightPosition[0]);
        glm::mat4 modelMatrix2 = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));
        GLint modelMatrixId2 = glGetUniformLocation(basicShader, "ModelMatrix");
        glUniformMatrix4fv(modelMatrixId2, 1, GL_FALSE, &modelMatrix2[0][0]);
        GLint projectionMatrixId3 = glGetUniformLocation(basicShader, "ProjectionMatrix");
        glUniformMatrix4fv(projectionMatrixId3, 1, GL_FALSE, &projectionMatrix[0][0]);
        GLint viewMatrixId3 = glGetUniformLocation(basicShader, "ViewMatrix");
        glUniformMatrix4fv(viewMatrixId3, 1, GL_FALSE, &viewMatrix[0][0]);
        glBindVertexArray(armadilloVAO);
        glDrawElements(GL_TRIANGLES, aIndices.size(), GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(skyboxShader);
    glDeleteBuffers(1, &cubemapVAO);
    glDeleteBuffers(1, &cubemapVBO);
    glDeleteBuffers(1, &cubemapIBO);

    glfwTerminate();
    return 0;
}