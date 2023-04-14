//
// Created by rwill on 4/14/2023.
//
#include <iostream>
#include <Box2D/Box2D.h>
#include <Box2D/Particle/b2ParticleSystem.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "creature.h"
#include "rendering.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Camera position
float cameraX = 0.0f;
float cameraY = 0.0f;
// Keyboard state
bool keys[GLFW_KEY_LAST] = {false};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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


GLFWwindow* initGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Get the dimensions of the primary monitor
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(primary_monitor);
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

    // Set up OpenGL
    setupOpenGL(window_width, window_height);

    return window;
}


void drawScene(const std::list<Creature *> &creatureList, b2ParticleSystem *particleSystem) {

    updateCamera();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-cameraX, -cameraY, 0);


    // Clear the screen
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the dynamic box
    for (Creature *creature: creatureList) {
        for (b2Body *creatureBody: creature->getBodyParts()) {
            drawCreature(creatureBody, creature->getHealth());
        }
    }

    glPointSize(3.0f);

    // Draw the particles
    glColor3f(0, 0, 1);
    glBegin(GL_POINTS);
    for (int i = 0; i < particleSystem->GetParticleCount(); ++i) {
        b2Vec2 particlePosition = particleSystem->GetPositionBuffer()[i];
        glVertex2f(particlePosition.x, particlePosition.y);
    }
    glEnd();

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

void drawCreature(const b2Body *body, float health) {
    auto *bodyData = static_cast<BodyData *>(body->GetUserData());

    if (bodyData) {
        glColor3f(bodyData->r, (bodyData->g * health) / 200, bodyData->b);
    } else {
        glColor3f(1, 0, 0);
    }

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
        } else {
            // Handle other shape types if needed (e.g., b2Shape::e_circle)
        }
    }

    glPopMatrix();
}