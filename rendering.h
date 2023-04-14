//
// Created by rwill on 4/14/2023.
//

#ifndef LIQUIDFUN_EVO_SIM_RENDERING_H
#define LIQUIDFUN_EVO_SIM_RENDERING_H

#include "creature.h"
#include <vector>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <Box2D/Particle/b2ParticleSystem.h>
#include <Box2D/Box2D.h>
#include <iostream>


void cleanUpScene();
void drawScene(const std::list<Creature *> &creatureList, b2ParticleSystem *particleSystem, float worldSize);
void drawCreature(const b2Body *body, float d);
GLFWwindow* initGLFW();

#endif //LIQUIDFUN_EVO_SIM_RENDERING_H
