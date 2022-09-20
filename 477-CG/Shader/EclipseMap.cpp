#include "EclipseMap.h"

using namespace std;

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;

    vertex() {}

    vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texture) : position(position),
                                                                                           normal(normal),
                                                                                           texture(texture) {}
};

struct triangle {
    int vertex1;
    int vertex2;
    int vertex3;

    triangle() {}

    triangle(const int &vertex1, const int &vertex2, const int &vertex3) : vertex1(vertex1), vertex2(vertex2),
                                                                           vertex3(vertex3) {}
};

void EclipseMap::Render(const char *coloredTexturePath, const char *greyTexturePath, const char *moonTexturePath) {
    // Open window
    GLFWwindow *window = openWindow(windowName, screenWidth, screenHeight);

    // Load shaders
    GLuint shaderID = initShaders("worldShader.vert", "worldShader.frag");
    GLuint moonShaderID = initShaders("moonShader.vert", "moonShader.frag");

    initColoredTexture(coloredTexturePath, shaderID);
    initGreyTexture(greyTexturePath, shaderID);

    initMoonColoredTexture(moonTexturePath, moonShaderID);

    setWorldVertices();
    setWorldIndices();
    configureWorldBuffers();

    setMoonVertices(glm::vec3(0,2600,0));
    setMoonIndices();
    configureMoonBuffers();


    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Main rendering loop
    do {
        glViewport(0, 0, screenWidth, screenHeight);

        glClearStencil(0);
        glClearDepth(1.0f);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glLoadIdentity();

        textureOffset -= ANGLE;
        if (textureOffset > 2*PI) textureOffset -= 2*PI;
        orbitDegree += ORBIT_ANGLE;
        if (orbitDegree > 2*PI) orbitDegree -= 2*PI;

        glUseProgram(shaderID);
        // Update camera at every frame
        updateCamera(shaderID);

        // Update uniform variables at every frame
        updateUniforms(shaderID);

        // Handle key presses
        handleKeyPress(window);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureGrey);

        // Use shaderID program
        glUseProgram(shaderID);

        glBindVertexArray(VAO);

        // Draw
        glDrawElements(GL_TRIANGLES, worldIndices.size(), GL_UNSIGNED_INT, 0);

        glLoadIdentity();

        glUseProgram(moonShaderID);
        updateCamera(moonShaderID);

        // Update uniform variables at every frame
        updateUniforms(moonShaderID);
        GLint degreeId = glGetUniformLocation(moonShaderID, "orbitDegree");
        glUniform1f(degreeId, orbitDegree);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTextureColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Use shaderID program
        glUseProgram(moonShaderID);

        glBindVertexArray(moonVAO);

        moonAngle -= ANGLE;
        if (moonAngle <= 0) moonAngle += 2*PI;

        glDrawElements(GL_TRIANGLES, moonIndices.size(), GL_UNSIGNED_INT, 0);


        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

    // Delete buffers
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderID);

    // Close window
    glfwTerminate();
}

void EclipseMap::setWorldVertices()
{
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / horizontalSplitCount;
    float stackStep = PI / verticalSplitCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= verticalSplitCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= horizontalSplitCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            worldVertices.push_back(x);
            worldVertices.push_back(y);
            worldVertices.push_back(z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            worldVertices.push_back(nx);
            worldVertices.push_back(ny);
            worldVertices.push_back(nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / horizontalSplitCount;
            t = (float)i / verticalSplitCount;
            worldVertices.push_back(s);
            worldVertices.push_back(t);
        }
    }
}

void EclipseMap::setWorldIndices()
{
    for (int i = 0; i < verticalSplitCount; i++) {
        int offset = 0;
        for (int j = 0; j < horizontalSplitCount; j++) {
            if (i != 0) {
                worldIndices.push_back(i * (horizontalSplitCount + 1) + offset);
                worldIndices.push_back((i + 1) * (horizontalSplitCount + 1) + offset);
                worldIndices.push_back(i * (horizontalSplitCount + 1) + 1 + offset);
            }
            if (i != verticalSplitCount - 1) {
                worldIndices.push_back(i * (horizontalSplitCount + 1) + 1 + offset);
                worldIndices.push_back((i + 1) * (horizontalSplitCount + 1) + offset);
                worldIndices.push_back((i + 1) * (horizontalSplitCount + 1) + 1 + offset);
            }
            offset++;
        }
    }
}

void EclipseMap::configureWorldBuffers()
{
    // Configure Buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, worldVertices.size() * sizeof(float), worldVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, worldIndices.size() * sizeof(float), worldIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void EclipseMap::setMoonVertices(glm::vec3 center)
{
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / moonRadius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / horizontalSplitCount;
    float stackStep = PI / verticalSplitCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= verticalSplitCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = moonRadius * cosf(stackAngle);             // r * cos(u)
        z = moonRadius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= horizontalSplitCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            moonVertices.push_back(center.x + x);
            moonVertices.push_back(center.y + y);
            moonVertices.push_back(center.z + z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            moonVertices.push_back(nx);
            moonVertices.push_back(ny);
            moonVertices.push_back(nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / horizontalSplitCount;
            t = (float)i / verticalSplitCount;
            moonVertices.push_back(s);
            moonVertices.push_back(t);
        }
    }
}

void EclipseMap::setMoonIndices()
{
    for (int i = 0; i < verticalSplitCount; i++) {
        int offset = 0;
        for (int j = 0; j < horizontalSplitCount; j++) {
            if (i != 0) {
                moonIndices.push_back(i * (horizontalSplitCount + 1) + offset);
                moonIndices.push_back((i + 1) * (horizontalSplitCount + 1) + offset);
                moonIndices.push_back(i * (horizontalSplitCount + 1) + 1 + offset);
            }
            if (i != verticalSplitCount - 1) {
                moonIndices.push_back(i * (horizontalSplitCount + 1) + 1 + offset);
                moonIndices.push_back((i + 1) * (horizontalSplitCount + 1) + offset);
                moonIndices.push_back((i + 1) * (horizontalSplitCount + 1) + 1 + offset);
            }
            offset++;
        }
    }
}

void EclipseMap::configureMoonBuffers()
{
    // Configure Buffers
    glGenVertexArrays(1, &moonVAO);
    glGenBuffers(1, &moonVBO);
    glGenBuffers(1, &moonEBO);

    glBindVertexArray(moonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
    glBufferData(GL_ARRAY_BUFFER, moonVertices.size() * sizeof(float), moonVertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, moonIndices.size() * sizeof(float), moonIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void EclipseMap::handleKeyPress(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 cameraLeft = glm::cross(cameraDirection, cameraUp);
        cameraUp = glm::rotate(cameraUp, 0.01f, cameraLeft);
        cameraDirection = glm::rotate(cameraDirection, 0.01f, cameraLeft);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 cameraLeft = glm::cross(cameraDirection, cameraUp);
        cameraUp = glm::rotate(cameraUp, -0.01f, cameraLeft);
        cameraDirection = glm::rotate(cameraDirection, -0.01f, cameraLeft);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraDirection = glm::rotate(cameraDirection, -0.01f, cameraUp);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraDirection = glm::rotate(cameraDirection, 0.01f, cameraUp);
    }


    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        speed += 0.02;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        speed -= 0.02;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        speed = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        heightFactor += 10;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        heightFactor -= 10;
    }


    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        cameraPosition = cameraStartPosition;
        cameraDirection = cameraStartDirection;
        cameraUp = cameraStartUp;
        pitch = startPitch;
        yaw = startYaw;
        speed = startSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {       
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {

            if (displayFormat == displayFormatOptions::windowed) {
                const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
                screenWidth = mode->width;
                screenHeight = mode->height;
                displayFormat = displayFormatOptions::fullScreen;
                glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, screenWidth, screenHeight, mode->refreshRate);
            } else if (displayFormat == displayFormatOptions::fullScreen) {
                const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
                screenWidth = defaultScreenWidth;
                screenHeight = defaultScreenHeight;
                displayFormat = displayFormatOptions::windowed;
                glfwSetWindowMonitor(window, NULL, 1, 31, screenWidth, screenHeight, mode->refreshRate);
            }
            pKeyPressed = false;
        }
    }
}

void EclipseMap::initMoonColoredTexture(const char *filename, GLuint shader) {
    int width, height;
    glGenTextures(1, &moonTextureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, moonTextureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
   

    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "MoonTexColor"), 2);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

GLFWwindow *EclipseMap::openWindow(const char *windowName, int width, int height) {
    if (!glfwInit()) {
        getchar();
        return 0;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, windowName, NULL, NULL);
    glfwSetWindowMonitor(window, NULL, 1, 31, screenWidth, screenHeight, mode->refreshRate);

    if (window == NULL) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0, 0, 0, 0);

    return window;
}

void EclipseMap::updateCamera(GLuint shaderID) {
    cameraPosition += speed * cameraDirection;


    glm::mat4 projectionMatrix = glm::perspective(glm::radians(projectionAngle), aspectRatio, near_, far_);

    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);

    glm::mat4 MVPMatrix = projectionMatrix * viewMatrix;

    glm::mat4 MVPMatrixNormal = inverseTranspose(viewMatrix);

    GLint projectionMatrixId = glGetUniformLocation(shaderID, "ProjectionMatrix");
    glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);

    GLint viewMatrixId = glGetUniformLocation(shaderID, "ViewMatrix");
    glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);

    GLint MVPMatrixId = glGetUniformLocation(shaderID, "MVP");
    glUniformMatrix4fv(MVPMatrixId, 1, GL_FALSE, &MVPMatrix[0][0]);

    GLint MVPMatrixNormalId = glGetUniformLocation(shaderID, "NormalMatrix");
    glUniformMatrix4fv(MVPMatrixNormalId, 1, GL_FALSE, &MVPMatrixNormal[0][0]);
}

void EclipseMap::updateUniforms(GLuint shaderID) {
    GLint cameraPositionId = glGetUniformLocation(shaderID, "cameraPosition");
    glUniform3fv(cameraPositionId, 1, &cameraPosition[0]);

    GLint lightPositionId = glGetUniformLocation(shaderID, "lightPosition");
    glUniform3fv(lightPositionId, 1, &lightPos[0]);

    GLint heightFactorId = glGetUniformLocation(shaderID, "heightFactor");
    glUniform1f(heightFactorId, this->heightFactor);


    GLint imageWidthId = glGetUniformLocation(shaderID, "imageWidth");
    glUniform1f(imageWidthId, this->imageWidth);

    GLint imageHeightId = glGetUniformLocation(shaderID, "imageHeight");
    glUniform1f(imageHeightId, this->imageHeight);

    GLint textureOffsetId = glGetUniformLocation(shaderID, "textureOffset");
    glUniform1f(textureOffsetId, this->textureOffset);
}

void EclipseMap::initColoredTexture(const char *filename, GLuint shader) {

    cout << shader << endl;
    int width, height;
    glGenTextures(1, &textureColor);
    glBindTexture(GL_TEXTURE_2D, textureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
   

    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "TexColor"), 0);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

void EclipseMap::initGreyTexture(const char *filename, GLuint shader) {

    glGenTextures(1, &textureGrey);
    glBindTexture(GL_TEXTURE_2D, textureGrey);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height;

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
  



    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "TexGrey"), 1);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

