//
// Created by rwill on 4/14/2023.
//
#include <GL/glew.h>
#include <iostream>
#include <Box2D/Box2D.h>
#include <Box2D/Particle/b2ParticleSystem.h>
#include "rendering.h"
// Include necessary headers
#include <iostream>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Camera position
float cameraX = 0.0f;
float cameraY = 0.0f;
// Keyboard state
bool keys[GLFW_KEY_LAST] = {false};


// Function prototypes
GLuint createShader(GLenum type, const char *source);

GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource);


// Vertex shader source code
const char *passThroughVertexShaderSource = R"(
    #version 120
    attribute vec2 aPosition;
    void main() {
        gl_Position = gl_ModelViewProjectionMatrix * vec4(aPosition, 0.0, 1.0);
    }
)";


const char *passThroughFragmentShaderSource = R"(
    #version 120
    void main() {
float Pi = 6.28318530718; // Pi*2

    // GAUSSIAN BLUR SETTINGS {{{
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 3.0; // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size = 8.0; // BLUR SIZE (Radius)
    // GAUSSIAN BLUR SETTINGS }}}

    vec2 Radius = Size/iResolution.xy;

    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    // Pixel colour
    vec4 Color = texture(iChannel0, uv);

    // Blur calculations
    for( float d=0.0; d<Pi; d+=Pi/Directions)
    {
		for(float i=1.0/Quality; i<=1.0; i+=1.0/Quality)
        {
			Color += texture( iChannel0, uv+vec2(cos(d),sin(d))*Radius*i);
        }
    }

    // Output to screen
    Color /= Quality * Directions - 15.0;
    fragColor =  Color;
        gl_FragColor = vec4(0, 0, 1, 1); // Set a fixed color for particles (blue)
    }
)";

const char *testfragmentShaderSource = R"(
    #version 120
    uniform sampler2D uTexture;
    varying vec2 vTexCoord;
    void main() {
        vec4 textureColor = texture2D(uTexture, vTexCoord);
        gl_FragColor = textureColor;
    }
)";


GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

void updateCamera() {
    float cameraSpeed = 0.1f; // Adjust this value to change the camera movement speed

    if (keys[GLFW_KEY_W]) {
        cameraY += cameraSpeed;
    }
    if (keys[GLFW_KEY_S]) {
        cameraY -= cameraSpeed;
    }
    if (keys[GLFW_KEY_A]) {
        cameraX -= cameraSpeed;
    }
    if (keys[GLFW_KEY_D]) {
        cameraX += cameraSpeed;
    }
}

void setupOpenGL(int screenWidth, int screenHeight) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    if (aspectRatio > 1.0f) {
        // Screen is wider than it is tall, adjust the x-coordinate range
        glOrtho(0, 25 * aspectRatio, 0, 25, -1, 1);
    } else {
        // Screen is taller than it is wide, adjust the y-coordinate range
        glOrtho(0, 25, 0, 25 / aspectRatio, -1, 1);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


GLFWwindow *initGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Get the dimensions of the primary monitor
    GLFWmonitor *primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *vidmode = glfwGetVideoMode(primary_monitor);
    int desktop_width = vidmode->width;
    int desktop_height = vidmode->height;

    // Calculate the window dimensions based on the desktop dimensions
    int window_width = desktop_width * 0.9;  // 90% of desktop width
    int window_height = desktop_height * 0.8;  // 90% of desktop height

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "LiquidFun Example", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    // Center the window on the screen
    int window_pos_x = (desktop_width - window_width) / 2;
    int window_pos_y = (desktop_height - window_height) / 2;
    glfwSetWindowPos(window, window_pos_x, window_pos_y);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, keyCallback);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        exit(1);
    }

    // Set up OpenGL
    setupOpenGL(window_width, window_height);

//    shaderProgram = createShaderProgram(passThroughVertexShaderSource, testfragmentShaderSource);

    return window;
}

void drawWorldBoundaries(float squareWidth) {

    float left = 0, right = squareWidth, top = squareWidth, bottom = 0;

    // Draw top edge
    glBegin(GL_LINES);
    glVertex2f(left, top);
    glVertex2f(right, top);
    glEnd();

    // Draw bottom edge
    glBegin(GL_LINES);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glEnd();

    // Draw left edge
    glBegin(GL_LINES);
    glVertex2f(left, bottom);
    glVertex2f(left, top);
    glEnd();

    // Draw right edge
    glBegin(GL_LINES);
    glVertex2f(right, bottom);
    glVertex2f(right, top);
    glEnd();
}

void cleanUpScene() {

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
}

void drawScene(const std::list<Creature *> &creatureList, b2ParticleSystem *particleSystem, float worldSize) {

    updateCamera();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-cameraX, -cameraY, 0);


    // Clear the screen
    glClearColor(0.8, 0.8, 0.8, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw world boundaries
    glColor3f(0, 0, 0);
    drawWorldBoundaries(worldSize);

    // Draw the dynamic box
    for (Creature *creature: creatureList) {
        for (b2Body *creatureBody: creature->getBodyParts()) {
            drawCreature(creatureBody, creature->getHealth());
        }
    }

    glPointSize(3.0f);
    glColor3f(0, 0, 1);
    // Use the shader program
//    glUseProgram(shaderProgram);

    // Draw the particles
    glBegin(GL_POINTS);
    for (int i = 0; i < particleSystem->GetParticleCount(); ++i) {
        b2Vec2 particlePosition = particleSystem->GetPositionBuffer()[i];
        glVertex2f(particlePosition.x, particlePosition.y);
    }
    glEnd();

    // Disable the shader program
//    glUseProgram(0);

}

// Create and compile a shader
GLuint createShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Error compiling shader: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// Create a shader program by linking vertex and fragment shaders
GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource) {
    vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glBindAttribLocation(program, 0, "aPosition");
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void drawPolygon(const b2PolygonShape *polygon, float scale = 1.0f) {
    int vertexCount = polygon->GetVertexCount();
    glBegin(GL_POLYGON);
    for (int i = 0; i < vertexCount; ++i) {
        b2Vec2 vertex = polygon->GetVertex(i) * scale;
        glVertex2f(vertex.x, vertex.y);
    }
    glEnd();
}

void drawCircle(const b2CircleShape *circle, float scale = 1.0f, int numSegments = 32) {
    float radius = circle->m_radius * scale;
    b2Vec2 center = circle->m_p * scale;
    float angleIncrement = 2 * M_PI / numSegments;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(center.x, center.y);

    for (int i = 0; i <= numSegments; ++i) {
        float angle = i * angleIncrement;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

void drawCreature(const b2Body *body, float health) {
    glColor3f(0, health / 1000, 0);

    glPushMatrix();
    b2Vec2 boxPosition = body->GetPosition();
    float32 boxAngle = body->GetAngle();
    glTranslatef(boxPosition.x, boxPosition.y, 0);
    glRotatef(boxAngle * 180.0f / M_PI, 0, 0, 1);

    // Iterate through the fixtures
    for (b2Fixture *fixture = const_cast<b2Fixture *>(body->GetFixtureList()); fixture; fixture = fixture->GetNext()) {
        b2Shape::Type shapeType = fixture->GetType();

        if (shapeType == b2Shape::e_polygon) {
            // Draw the polygon shape
            b2PolygonShape *polygonShape = dynamic_cast<b2PolygonShape *>(fixture->GetShape());
            drawPolygon(polygonShape);
        } else if (shapeType == b2Shape::e_circle) {
            b2CircleShape *circlShape = dynamic_cast<b2CircleShape *>(fixture->GetShape());
            drawCircle(circlShape);
            // Handle other shape types if needed (e.g., b2Shape::e_circle)
        }
    }

    glPopMatrix();
}