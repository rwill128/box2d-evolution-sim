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
#include <regex>

class Creature;

class Creature {
private:
    float health;
    float mass;
    std::string m_geneticCode;

    void decodeGeneticCode(const std::string &code) {
        std::istringstream input(code);
        char ch;

        b2Body *currentBody = nullptr;

        while (input >> ch) {
            switch (ch) {
                case 'B': {
                    float x, y, angle;
                    char type;
                    input >> ch; // Ignore opening parenthesis
                    input >> x >> ch >> y >> ch >> type >> ch >> angle;
                    input >> ch; // Ignore closing parenthesis
                    input >> ch; // Ignore the |
                    currentBody = createBody(x, y, type, angle);
                }
                    break;

                case 'F': {
                    if (currentBody) {
                        char shapeType;
                        float density, friction, restitution;
                        input >> ch; // Ignore opening parenthesis
                        input >> shapeType;


                        switch (shapeType) {
                            case 'P': { // Polygon shape
                                std::vector<b2Vec2> vertices;
                                float x, y;

                                input >> ch; // Ignore opening bracket

                                while (ch != ']') {
                                    input >> ch; // Ignore opening parenthesis
                                    input >> x;
                                    input >> ch;
                                    input >> y;
                                    input >> ch; // Ignore closing parenthesis
                                    input >> ch; // Read the next character (comma or closing bracket)
                                    vertices.push_back(b2Vec2(x, y));
                                }

                                input >> ch; // Ignore closing bracket

                                b2PolygonShape polygonShape;
                                polygonShape.Set(vertices.data(), vertices.size());

                                b2FixtureDef fixtureDef;
                                fixtureDef.shape = &polygonShape;

                                input >> density;
                                input >> ch;
                                input >> friction;
                                input >> ch;
                                input >> restitution;
                                input >> ch; // Ignore closing parenthesis
                                input >> ch; // Ignore the |

                                fixtureDef.density = density;
                                fixtureDef.friction = friction;
                                fixtureDef.restitution = restitution;

                                currentBody->CreateFixture(&fixtureDef);
                            }
                                break;

                            case 'C': { // Circle shape
                                float centerX, centerY, radius;
                                input >> ch; // Ignore opening parenthesis
                                input >> centerX;
                                input >> ch;
                                input >> centerY;
                                input >> ch;
                                input >> radius;
                                input >> ch; // Ignore closing parenthesis
                                input >> ch; // Ignore closing separating semicolon

                                b2CircleShape circleShape;
                                circleShape.m_p.Set(centerX, centerY);
                                circleShape.m_radius = radius;

                                b2FixtureDef fixtureDef;
                                fixtureDef.shape = &circleShape;

                                input >> density;
                                input >> ch;
                                input >> friction;
                                input >> ch;
                                input >> restitution;
                                input >> ch; // Ignore closing parenthesis
                                input >> ch; // Ignore the |

                                fixtureDef.density = density;
                                fixtureDef.friction = friction;
                                fixtureDef.restitution = restitution;

                                currentBody->CreateFixture(&fixtureDef);
                            }
                                break;

//                            case 'E': { // Edge shape
//                                float x1, y1, x2, y2;
//                                input >> ch; // Ignore opening parenthesis
//                                input >> x1;
//                                input >> ch;
//                                input >> y1;
//                                input >> ch;
//                                input >> x2;
//                                input >> ch;
//                                input >> y2;
//                                input >> ch; // Ignore closing parenthesis
//                                input >> ch; // Skip semicolon
//
//                                b2EdgeShape edgeShape;
//                                edgeShape.Set(b2Vec2(x1, y1), b2Vec2(x2, y2));
//                                edgeShape.m_type
//
//                                b2FixtureDef fixtureDef;
//                                fixtureDef.shape = &edgeShape;
//
//                                input >> density;
//                                input >> ch;
//                                input >> friction;
//                                input >> ch;
//                                input >> restitution;
//                                input >> ch; // Ignore closing parenthesis
//
//                                fixtureDef.density = density;
//                                fixtureDef.friction = friction;
//                                fixtureDef.restitution = restitution;
//
//                                currentBody->CreateFixture(&fixtureDef);
//                            }
//                                break;

                            default:
                                std::cerr << "Unknown shape type: " << shapeType << std::endl;
                                return;
                        }


                    } else {
                        std::cerr << "Trying to create a fixture without a body." << std::endl;
                    }
                }
                    break;

                    // Other cases...

                default:
                    std::cerr << "Unknown structure type: " << ch << std::endl;
                    break;
            }
        }
    }

    b2Body *createBody(float x, float y, char type, float angle) {
        b2BodyDef bodyDef;
        bodyDef.position.Set(x, y);
        bodyDef.angle = angle * b2_pi / 180.0f; // Convert to radians

        switch (type) {
            case 'D':
                bodyDef.type = b2_dynamicBody;
                break;
            case 'S':
                bodyDef.type = b2_staticBody;
                break;
            case 'K':
                bodyDef.type = b2_kinematicBody;
                break;
            default:
                std::cerr << "Unknown body type: " << type << std::endl;
                return nullptr;
        }

        b2Body *body = m_world->CreateBody(&bodyDef);
        body->SetUserData(this);

        m_bodies.push_back(body);

        return body;
    }

    b2World *m_world;
    std::vector<b2Body *> m_bodies;

public:


    std::string stripCurlyBraces(const std::string &input) {
        // Define the regular expression to match everything inside curly braces.
        const std::regex pattern("\\{[^\\}]*\\}");

        // Replace all occurrences of the regular expression with an empty string.
        return std::regex_replace(input, pattern, "");
    }


    Creature(const std::string &geneticCode, b2World *world) : m_geneticCode(geneticCode), m_world(world) {
        health = 100;
        mass = 0.0f;

        decodeGeneticCode(stripCurlyBraces(geneticCode));

        for (b2Body *body: m_bodies) {
            mass += body->GetMass();
        }
    }

    Creature() : health(100.0f) {}

    void addToHealth(float h) { health += h; }

    void incurTimeStepCost(float costMultiplier) {
        health -= costMultiplier * mass;
    }

    float getHealth() const { return health; }

    float getMass() const { return mass; }

    std::string getGeneticCode() const { return m_geneticCode; }

    const std::vector<b2Body *> &getBodyParts() const { return m_bodies; }

};


#endif //LIQUIDFUN_EVO_SIM_CREATURE_H
