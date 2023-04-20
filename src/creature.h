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

class Creature;

struct BodyData {
    // Color components: red, green, blue, alpha
    float r, g, b, a;
    Creature *parentCreature;

    BodyData(float red, float green, float blue, float alpha)
            : r(red), g(green), b(blue), a(alpha) {}
};

class Creature {
private:
    float health;
    float offsetX; // offset values for reproduction
    float offsetY;
    std::list<b2Body *> bodyParts; // assuming Box2D bodies make up the creature's body
public:
    Creature(std::list<b2Body *> vector) {
        health = 100.0f;
        bodyParts = std::move(vector);
        offsetX = 2.0f;
        offsetY = 2.0f;
    }

    Creature() : health(100.0f), offsetX(2.0f), offsetY(2.0f) {}

    void setHealth(float h) { health = h; }

    void addToHealth(float h) { health += h; }

    float getHealth() const { return health; }

    const std::list<b2Body *> &getBodyParts() const { return bodyParts; }

    void addBodyPart(b2Body *body) { bodyParts.push_back(body); }

    static b2Body *Creature::createBodyPart(b2World *world, Creature* parentCreature, float x, float y, float width, float height) {
        // Create the body definition
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(x, y);

        // Create the body in the world
        b2Body *body = world->CreateBody(&bodyDef);

        // Create the shape
        b2PolygonShape dynamicBox;
        dynamicBox.SetAsBox(width / 2.0f, height / 2.0f);

        // Create the fixture
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;

        // Attach the fixture to the body
        body->CreateFixture(&fixtureDef);

        // Create the BodyData object with the specified color
        auto *bodyData = new BodyData(0.0f, 1.0f, 0.0f, 1.0f);

        bodyData->parentCreature = parentCreature;

        // Set the user data of the b2Body
        body->SetUserData(bodyData);

        // Add the body to the creature
        return body;
    }


    Creature *reproduce(b2World *world, float mutationRate = 0.1) const;

    // Getter and setter functions for offsetX and offsetY
    float getOffsetX() const { return offsetX; }

    void setOffsetX(float x) { offsetX = x; }

    float getOffsetY() const { return offsetY; }

    void setOffsetY(float y) { offsetY = y; }
};


#endif //LIQUIDFUN_EVO_SIM_CREATURE_H
