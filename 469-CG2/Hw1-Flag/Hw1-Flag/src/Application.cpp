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

typedef struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    Vertex()
    {
        position = glm::vec3(0);
        normal = glm::vec3(0);
        texCoord = glm::vec2(0);
    }
} Vertex;

typedef struct Patch
{
    vector<float> controlPoints;
} Patch;

float proectionAngle = 45.0f;
float aspectRatio = 1024.0f/768.0f;
float near = 0.1f;
float far = 1000.0f;
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 lightPosition2 = glm::vec3(10.0f, 10.0f, 0.0f);

int nr_patches = 1;
int sample = 10;
float l = -9.0f;
float r = 9.0f;
float b = -6.0f;
float t = 6.0f;
float z = -30.0f;

float angle = 0;

vector<Patch> patches;
vector<Vertex> sampleVertices;
vector<unsigned int> indices;


void GenerateControlPoints(int np, float l, float r, float t, float b, float z)
{
    for (int i = 0; i < patches.size(); i++)
    {
        patches[i].controlPoints.clear();
    }
    patches.clear();

    float hinc = (r - l) / (np * 3);
    float vinc = (t - b) / (np * 3);

    for (int i = 0; i < np; i++)
    {
        for (int j = 0; j < np; j++)
        {
            Patch patch;

            float local_l = l + j * 3 * hinc;
            float local_t = t - i * 3 * vinc;

            for (int ci = 0; ci < 4; ci++)
            {
                for (int cj = 0; cj < 4; cj++)
                {
                    patch.controlPoints.push_back(local_l + cj * hinc);
                    patch.controlPoints.push_back(local_t - ci * vinc);
                    patch.controlPoints.push_back(z);
                }
            }

            patches.push_back(patch);
        }
    }
}


int factorial(int n)
{
    int res = 1;
    while (n > 1)
    {
        res *= n;
        n--;
    }
    return res;
}   

int choose(int n, int k)
{
    if (k > n) return 0;
    if (k * 2 > n) k = n - k;
    if (k == 0) return 1;

    int result = n;
    for (int i = 2; i <= k; i++)
    {
        result *= (n - i + 1);
        result /= i;
    }

    return result;
}

float bern(int i, int n, float t)
{
    return choose(n, i) * pow(t, i) * pow(1 - t, n - i);
}

Vertex* Q(float s, float t, vector<float> cp)
{
    Vertex *ver = new Vertex();

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ver->position.x += bern(i, 3, s) * bern(j, 3, t) * cp[(4 * i + j) * 3];
            ver->position.y += bern(i, 3, s) * bern(j, 3, t) * cp[(4 * i + j) * 3 + 1];
            ver->position.z += bern(i, 3, s) * bern(j, 3, t) * cp[(4 * i + j) * 3 + 2];
        }
    }

    return ver;
}

void GenerateSamples()
{
    sampleVertices.clear();

    float increment = 1.0f / (sample-1);
    float s = 0.0f, t = 0.0f;

    for (int n = 0; n < nr_patches; n++)  // Row of patch
    {
        for (int i = 0; i < sample; i++) // s coord
        {
            if (n > 0 && s == 0)
            {
                s += increment;
                continue;
            }

            for (int p = 0; p < nr_patches; p++) // Col of patch
            {
                for (int j = 0; j < sample; j++) // t coord
                {
                    if (p > 0 && j == 0)
                    {
                        t += increment;
                        continue;
                    }

                    sampleVertices.push_back(*Q(s, t, patches[n*nr_patches + p].controlPoints));
                    t += increment;
                }
                t = 0.0f;
            }
            s += increment;
        }
        s = 0.0f;
    }

    //cout << "Vertex count: " << sampleVertices.size() << endl;

    /*for (int p = 0; p < patches.size(); p++)
    {
        float s = 0.0f, t = 0.0f;
        for (int i = 0; i < sample; i++)
        {
            if (p >= nr_patches && i == 0) {
                s += increment;
                continue;
            }
            for (int j = 0; j < sample; j++)
            {
                if (p % nr_patches != 0 && j == 0) {
                    t += increment;
                    continue;
                }
                sampleVertices.push_back(*Q(s, t, patches[p].controlPoints));
                t += increment;
            }
            s += increment;
            t = 0.0f;
        }
    }*/
}

void GenerateIndices()
{
    indices.clear();

    int n = (sample - 1) * nr_patches + 1;

    for (int i = 0; i < n - 1; i++)
    {
        indices.push_back(i);
        indices.push_back(i + n);
        indices.push_back(i + 1);
    }
    for (int i = 1; i < n - 1; i++)
    {
        for (int j = 0; j < n - 1; j++)
        {
            indices.push_back(n * i + j);
            indices.push_back(n * i + j + 1);
            indices.push_back(n * i + j - (n - 1));

            indices.push_back(n * i + j);
            indices.push_back(n * i + j + n);
            indices.push_back(n * i + j + 1);
        }
    }
    for (int i = 0; i < n - 1; i++)
    {
        indices.push_back(n * (n - 1) + i);
        indices.push_back(n * (n - 1) + i + 1);
        indices.push_back(n * (n - 1) + i - (n - 1));
    }
}

void CalcNormals()
{
    int sampleSize = sampleVertices.size();
    for (int i = 0; i < sampleSize; i++)
    {
        sampleVertices[i].normal = glm::vec3(0);
    }

    int indicesSize = indices.size();
    for (int i = 0; i < indicesSize; i+=3)
    {
        glm::vec3 p1 = sampleVertices[indices[i]].position;
        glm::vec3 p2 = sampleVertices[indices[i+1]].position;
        glm::vec3 p3 = sampleVertices[indices[i+2]].position;

        glm::vec3 u = p2 - p1;
        glm::vec3 v = p3 - p1;
        glm::vec3 n = glm::vec3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);

        sampleVertices[indices[i]].normal += n;
        sampleVertices[indices[i+1]].normal += n;
        sampleVertices[indices[i+2]].normal += n;
    }

    for (int i = 0; i < sampleSize; i++)
    {
        sampleVertices[i].normal = glm::normalize(sampleVertices[i].normal);
        //cout << "Normal " << i << ": " << sampleVertices[i].normal.x << ", " << sampleVertices[i].normal.y << ", " << sampleVertices[i].normal.z << endl;
    }
}

void CalcTexCoords()
{
    int size = patches.size();

    float top = patches[0].controlPoints[1];
    float left = patches[0].controlPoints[0];
    float bottom = patches[size-1].controlPoints[15 * 3 + 1];
    float right = patches[size-1].controlPoints[15 * 3];
    
    int sampleSize = sampleVertices.size();
    for (int i = 0; i < sampleSize; i++)
    {
        float tx = abs(sampleVertices[i].position.x - left) / (right - left);
        float ty = 1.0 - abs(sampleVertices[i].position.y - bottom) / (top - bottom);

        sampleVertices[i].texCoord.x = tx;
        sampleVertices[i].texCoord.y = ty;
    }
}

void Animate()
{ // Z koordinatlar�n� yatayda sin �izecek �ekilde ayarla sinle hareket ettirme
    angle += 10;
    float rad = angle * 0.01745329252;

    int patchesSize = patches.size();
    for (int p = 0; p < patchesSize; p++)
    {
        for (int i = 5; i < 48; i += 12)
        {
            if (p % nr_patches != 0) {
                patches[p].controlPoints[i - 3] = patches[p - 1].controlPoints[i + 6];
                patches[p].controlPoints[i] = 2 * patches[p - 1].controlPoints[i + 6] - patches[p - 1].controlPoints[i + 3];
            }
            else {
                patches[p].controlPoints[i] = -30 + sin(rad) * 2;
            }
            patches[p].controlPoints[i + 3] = -30 + cos(rad) * 2;
            patches[p].controlPoints[i + 6] = -30 + sin(rad) * 2;
        }
    }   
}

void HandleKeyPress(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
            sample++;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
            sample = max(sample - 1, 2);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
        {
            nr_patches++;
            GenerateControlPoints(nr_patches, l, r, t, b, z);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
        {
            nr_patches = max(nr_patches - 1, 1);
            GenerateControlPoints(nr_patches, l, r, t, b, z);
        }
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

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 768, "Flag", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    if (glewInit() != GLEW_OK)
        std::cout << "Glew is not initialized!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;


    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int IBO;
    glGenBuffers(1, &IBO);

   
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int image_width, image_height, image_nrChannels;
    unsigned char* data = stbi_load("res\\textures\\michael.jpg", &image_width, &image_height, &image_nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture." << endl;
    }
    stbi_image_free(data);


    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    glUseProgram(shader);

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(proectionAngle), aspectRatio, near, far);
    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);

    GLint projectionMatrixId = glGetUniformLocation(shader, "ProjectionMatrix");
    glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);

    GLint viewMatrixId = glGetUniformLocation(shader, "ViewMatrix");
    glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);

    GLint cameraPositionId = glGetUniformLocation(shader, "CameraPosition");
    glUniform3fv(cameraPositionId, 1, &cameraPosition[0]);

    GLint lightPositionId = glGetUniformLocation(shader, "LightPosition");
    glUniform3fv(lightPositionId, 1, &lightPosition[0]);

    GLint lightPosition2Id = glGetUniformLocation(shader, "LightPosition2");
    glUniform3fv(lightPosition2Id, 1, &lightPosition2[0]);

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    GenerateControlPoints(nr_patches, l, r, t, b, z);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0)
        {
            string title = "FPS: " + to_string(nbFrames) + " Sample: " + to_string(sample) + " Patches: " + to_string(nr_patches);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }

        /* Render here */
        glClearColor(0.50, 0.81, 0.92, 1); // Background color
        glClear(GL_COLOR_BUFFER_BIT);

        GenerateSamples();
        GenerateIndices();
        CalcNormals();
        CalcTexCoords();
        Animate();

        HandleKeyPress(window);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sampleVertices.size() * 8 * sizeof(float), &sampleVertices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(6 * sizeof(float)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);

    glfwTerminate();
    return 0;
}