//
// Created by rwill on 4/12/2023.
//

#ifndef LIQUIDFUN_EVO_SIM_CREATURE_H
#define LIQUIDFUN_EVO_SIM_CREATURE_H

#include <list>
#include <Box2D/Dynamics/b2Body.h>
#include <utility>
#include <vector>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Box2D.h>
#include <Box2D/Particle/b2ParticleSystem.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

class Creature;

class Creature {
private:
    float health;
    std::vector<b2Body *> bodyParts;

    void decodeGeneticCode(const std::string& code) {
        std::istringstream input(code);
        char ch;

        b2Body* currentBody = nullptr;

        while (input >> ch) {
            switch (ch) {
                case 'B': {
                    float x, y, angle;
                    char type;
                    input.ignore(); // Ignore opening parenthesis
                    input >> x >> ch >> y >> ch >> type >> ch >> angle;
                    input.ignore(); // Ignore closing parenthesis
                    currentBody = createBody(x, y, type, angle);
                } break;

                case 'F': {
                    if (currentBody) {
                        char shapeType;
                        float density, friction, restitution;
                        input.ignore(); // Ignore opening parenthesis
                        input >> shapeType;

                        b2FixtureDef fixtureDef;

                        switch (shapeType) {
                            case 'P': { // Polygon shape
                                std::vector<b2Vec2> vertices;
                                char vertexChar;
                                float x, y;

                                input.ignore(); // Ignore opening bracket

                                while (input.peek() != ']') {
                                    input.ignore(); // Ignore opening parenthesis
                                    input >> x >> ch >> y;
                                    input.ignore(); // Ignore closing parenthesis
                                    input >> vertexChar; // Read the next character (comma or closing bracket)
                                    vertices.push_back(b2Vec2(x, y));
                                }

                                input.ignore(); // Ignore closing bracket

                                b2PolygonShape polygonShape;
                                polygonShape.Set(vertices.data(), vertices.size());
                                fixtureDef.shape = &polygonShape;
                            } break;

                            case 'C': { // Circle shape
                                float centerX, centerY, radius;
                                input.ignore(); // Ignore opening parenthesis
                                input >> centerX >> ch >> centerY >> ch >> radius;
                                input.ignore(); // Ignore closing parenthesis

                                b2CircleShape circleShape;
                                circleShape.m_p.Set(centerX, centerY);
                                circleShape.m_radius = radius;
                                fixtureDef.shape = &circleShape;
                            } break;

                            default:
                                std::cerr << "Unknown shape type: " << shapeType << std::endl;
                                return;
                        }

                        input >> ch >> density >> ch >> friction >> ch >> restitution;
                        input.ignore(); // Ignore closing parenthesis

                        fixtureDef.density = density;
                        fixtureDef.friction = friction;
                        fixtureDef.restitution = restitution;

                        currentBody->CreateFixture(&fixtureDef);
                    } else {
                        std::cerr << "Trying to create a fixture without a body." << std::endl;
                    }
                } break;

                    // Other cases...

                default:
                    std::cerr << "Unknown structure type: " << ch << std::endl;
                    break;
            }
        }
    }

    b2Body* createBody(float x, float y, char type, float angle) {
        b2BodyDef bodyDef;
        bodyDef.position.Set(x, y);
        bodyDef.angle = angle * b2_pi / 180.0f; // Convert to radians

        switch (type) {
            case 'D': bodyDef.type = b2_dynamicBody; break;
            case 'S': bodyDef.type = b2_staticBody; break;
            case 'K': bodyDef.type = b2_kinematicBody; break;
            default:
                std::cerr << "Unknown body type: " << type << std::endl;
                return nullptr;
        }

        b2Body* body = m_world->CreateBody(&bodyDef);
        m_bodies.push_back(body);

        return body;
    }

    b2World* m_world;
    std::vector<b2Body*> m_bodies;

public:

    Creature(const std::string& geneticCode, b2World* world): m_world(world) {
        decodeGeneticCode(geneticCode);
    }

    Creature() : health(100.0f) {}

    void addToHealth(float h) { health += h; }

    float getHealth() const { return health; }

    const std::vector<b2Body *> &getBodyParts() const { return bodyParts; }

};


#endif //LIQUIDFUN_EVO_SIM_CREATURE_H
