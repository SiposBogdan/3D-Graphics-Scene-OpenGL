#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#define NR_POINT_LIGHTS 5

#include <iostream>


std::vector<Keyframe> cameraPath = {
    {{-20.0f, 10.0f, 20.0f}, {1.0f, 0.0f, 0.0f}, 0.0f},
    {{20.0f, 10.0f, 20.0f}, {1.0f, 0.0f, 0.0f},10.0f},
    {{20.0f, 15.0f, -20.0f}, {0.0f, -1.0f, 0.0f}, 20.0f},
    {{-20.0f, 15.0f, -20.0f}, {-1.0f, 0.0f, 0.0f}, 30.0f}
};


glm::vec3 lightPositions[NR_POINT_LIGHTS] = {
    glm::vec3(20.2f, 8.0f, -19.2f),
    glm::vec3(14.6f, 8.5f, -20.0f),
    glm::vec3(4.2f, 8.6f, -19.7f),
    glm::vec3(-4.0f, 9.0f, -19.6f),
    glm::vec3(-17.4f, 9.0f, -18.7f)
};
glm::vec3 lightColors[NR_POINT_LIGHTS] = {
    glm::vec3(1.0f, 0.0f, 0.0f), // Red
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f)
};

glm::vec3 lanternPosition = glm::vec3(10.0f, 5.6f, -3.0f);


// window
gps::Window myWindow;
int glWindowWidth = 900;
int glWindowHeight = 700;
float lastX = glWindowWidth / 2;
float lastY = glWindowHeight / 2;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPunctiform;
glm::vec3 lightPunctiformColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint showCeataLoc;
GLint showLuminaPunctiformaLoc;
GLint showLuminaSpot;
GLuint shadowMapFBO;
GLuint depthMapTexture;

GLint lightPunctiformLoc;
GLint lightPositionsLoc[NR_POINT_LIGHTS];
GLint lightColorsLoc[NR_POINT_LIGHTS];
GLint lightPunctiformColorLoc;
GLint showLuminaSpotLoc;


//glm::vec3 spotlightPosition = glm::vec3(10.0, 6.4, -3.0); // Spotlight position
glm::vec3 spotlightPosition = lanternPosition;
glm::vec3 spotlightDirection = glm::vec3(0.0f, 0.0f, 1.0f); // Spotlight pointing downward
float spotlightCutoff = glm::cos(glm::radians(12.5f));  // Inner cone
float spotlightOuterCutoff = glm::cos(glm::radians(17.5f)); // Outer cone
float spotlightConstant = 1.0f;
float spotlightLinear = 0.09f;
float spotlightQuadratic = 0.032f;
glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 1.0f); // White light

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 10.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D flashlight;

gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;


GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader skyboxShader;
gps::Shader depthMap;

int retina_width, retina_height;

//shadow
const unsigned int Shadow_Width = 8192;
const unsigned int Shadow_Height = 8192;

float scalare = 1;

std::ifstream file("direction.txt");

float angleY = 0;
bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    //fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
    WindowDimensions a = { retina_width, retina_height };
    myWindow.setWindowDimensions(a);
    glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);
    myBasicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}



void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static float lastX = glWindowWidth / 2.0f; 
    static float lastY = glWindowHeight / 2.0f; 

    static const float sensitivity = 1.0f; 

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }


    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 


    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

 
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

float startTime = 0.0f;
int start = 1;
void processMovement() {

    if (pressedKeys[GLFW_KEY_1]) {
        angleY += 1.0f; 
        start = 1;
    }
    if (pressedKeys[GLFW_KEY_2]) {
        angleY -= 1.0f; 
        start = 1;
    }
    if (pressedKeys[GLFW_KEY_3]) {
        scalare += 0.01f;
        start = 1;
    }
    if (pressedKeys[GLFW_KEY_4]) {
        scalare -= 0.01f;
        if (scalare < 0.1f) scalare = 0.1f;
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_R]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_F]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

        start = 1;
    }


    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        start = 1;
    }


    if (pressedKeys[GLFW_KEY_C]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_V]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showCeataLoc, 1);
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_B]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showCeataLoc, 0);
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_H]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showLuminaPunctiformaLoc, 1);
        start = 1;
    }
    if (pressedKeys[GLFW_KEY_G]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showLuminaPunctiformaLoc, 0);
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_T]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showLuminaSpotLoc, 1);
        start = 1;
    }
    if (pressedKeys[GLFW_KEY_Y]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showLuminaSpotLoc, 0);
        start = 1;
    }

    if (pressedKeys[GLFW_KEY_0]) {
        start = 0;
        startTime = glfwGetTime();;
    }
}

void initOpenGLWindow() {
    myWindow.Create(glWindowWidth, glWindowHeight, "OpenGL Project Core");
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
    WindowDimensions a = { retina_width, retina_height };
    myWindow.setWindowDimensions(a);
    glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    nanosuit.LoadModel("models/scenaLaMare/nou.obj");
    ground.LoadModel("models/soccer/soccerball.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    flashlight.LoadModel("models/lanterna/flashlight.obj");

    /*faces.push_back("skybox/alpha-island_rt.tga");
    faces.push_back("skybox/alpha-island_lf.tga");
    faces.push_back("skybox/alpha-island_up.tga");
    faces.push_back("skybox/alpha-island_dn.tga");
    faces.push_back("skybox/alpha-island_bk.tga");
    faces.push_back("skybox/alpha-island_ft.tga");*/
    faces.push_back("skybox/lagoon_rt.tga");
    faces.push_back("skybox/lagoon_lf.tga");
    faces.push_back("skybox/lagoon_up.tga");
    faces.push_back("skybox/lagoon_dn.tga");
    faces.push_back("skybox/lagoon_bk.tga");
    faces.push_back("skybox/lagoon_ft.tga");
    mySkyBox.Load(faces);
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    //lightShader.useShaderProgram();
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    // screenQuadShader.useShaderProgram();
    depthMap.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms() {

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    myBasicShader.useShaderProgram();

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    view = myCamera.getViewMatrix();
    glm::mat4 scalar = glm::scale(glm::mat4(1.0f), glm::vec3(scalare));
    view = view * scalare;
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");


    projection = glm::perspective(glm::radians(60.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 50.0f);

    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));


    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");

    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    myBasicShader.useShaderProgram();
    showCeataLoc = glGetUniformLocation(myBasicShader.shaderProgram, "showCeata");

    myBasicShader.useShaderProgram();
    //lumina punctiforma
    /*lightPunctiform = glm::vec3(19.0f, 12.0f, -19.5f);
    lightPunctiformLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPunctiform");
    glUniform3fv(lightPunctiformLoc, 1, glm::value_ptr(lightPunctiform));
    lightPunctiformColor = glm::vec3(1.0f, 0.0f, 0.0f);
    lightPunctiformColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, " lightPunctiformColor");
    glUniform3fv(lightPunctiformColorLoc, 1, glm::value_ptr(lightPunctiformColor));*/
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        std::string posName = "punctiformLights[" + std::to_string(i) + "].position";
        //std::string colorName = "pointLights[" + std::to_string(i) + "].color";
        lightPositionsLoc[i] = glGetUniformLocation(myBasicShader.shaderProgram, posName.c_str());
        lightPunctiformColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, " lightPunctiformColor");
        //lightColorsLoc[i] = glGetUniformLocation(myBasicShader.shaderProgram, colorName.c_str());

        glUniform3fv(lightPositionsLoc[i], 1, glm::value_ptr(lightPositions[i]));
        glUniform3fv(lightPunctiformColorLoc, 1, glm::value_ptr(lightPunctiformColor));
    }

    showLuminaPunctiformaLoc = glGetUniformLocation(myBasicShader.shaderProgram, "showLuminaPunctiforma");


    myBasicShader.useShaderProgram();

    // Spotlight properties
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightPosition"), 1, glm::value_ptr(spotlightPosition));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightDirection"), 1, glm::value_ptr(glm::normalize(spotlightDirection)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightCutoff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightOuterCutoff"), glm::cos(glm::radians(17.5f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightConstant"), spotlightConstant);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightLinear"), spotlightLinear);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightQuadratic"), spotlightQuadratic);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlightColor"), 1, glm::value_ptr(spotlightColor));


    showLuminaSpotLoc = glGetUniformLocation(myBasicShader.shaderProgram, "showLuminaSpot");



}



void initFBOs()
{
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        Shadow_Width, Shadow_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
    const GLfloat near_plane = -20.0f, far_plane = 20.0f;
    glm::mat4 lightView = glm::lookAt(glm::inverseTranspose(glm::mat3(lightRotation)) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -60.0f, 60.0f, near_plane, far_plane);

    return lightProjection * lightView;

}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    //teapot.Draw(shader);
    //nanosuit.Draw(shader);
    //ground.Draw(shader);
}
float delta = 0;
float movementSpeed = 2; // units per second
void updateDelta(double elapsedSeconds) {
    delta = delta + movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();

void drawObjects(gps::Shader shader, bool depthPass) {

    shader.useShaderProgram();
    /*model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));*/
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(scalare)); // Aplică scalarea globală

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    nanosuit.Draw(shader);


    double currentTimeStamp = glfwGetTime();
    updateDelta(currentTimeStamp - lastTimeStamp);
    lastTimeStamp = currentTimeStamp;

    glm::vec3 ballPosition = glm::vec3(-70.0f + delta, 6.4f, -5.6f); 
    float rotationAngle = delta * 90.0f; 
    if (ballPosition.x > 24.0f) {
        ballPosition = glm::vec3(20, 6.4f, -5.6f); // Reset position
        delta = 50;
    }

    
    model = glm::translate(glm::mat4(1.0f), ballPosition);
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f)); 
    model = glm::scale(model, glm::vec3(0.1f) *scalare); 
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }


    ground.Draw(shader);


    //shader.useShaderProgram();
    model = glm::translate(glm::mat4(1.0f), lanternPosition);
    model = glm::scale(model, glm::vec3(0.1f)*scalare);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    flashlight.Draw(shader);
    mySkyBox.Draw(skyboxShader, view, projection);
}

void showShadow() {
    depthMap.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMap.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, Shadow_Width, Shadow_Height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMap, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene
    showShadow();
    // render the teapot
    glViewport(0, 0, retina_width, retina_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Activarea testului de adâncime
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Fragmentele mai aproape de cameră trec testul

    // Activarea eliminării fațetelor ascunse
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // Elimină fațetele din spate
    glFrontFace(GL_CCW); // Fațetele vizibile sunt în sens trigonometric invers

    // Modify the view matrix to include rotation around the X-axis
    

    myBasicShader.useShaderProgram();
    view = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f)) * myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));



    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(10.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    drawObjects(myBasicShader, false);


    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    //
    glm::vec3 adjustedLightDir = lightDir + glm::vec3(0.0f, 30.0f, 0.0f);
    model = lightRotation;
    model = glm::translate(model, 1.0f * adjustedLightDir);
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    lightCube.Draw(lightShader);

    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        glm::mat4 lightModels = glm::translate(glm::mat4(1.0f), lightPositions[i]);
        lightModels = glm::scale(lightModels, glm::vec3(0.2f)); 
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightModels));
        //lightCube.Draw(lightShader);
    }

}

void cleanup() {

    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    //close GL context and any other GLFW resources
    myWindow.Delete();
    //cleanup code for your own data
}



int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();
    initFBOs();

    glCheckError();

    while (!glfwWindowShouldClose(myWindow.getWindow())) {

        processMovement();
        if(start == 0)
            myCamera.moveFreely(startTime, glfwGetTime(), cameraPath);

        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
