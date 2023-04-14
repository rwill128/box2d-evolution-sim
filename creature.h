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
#include "creature.h"
#include "rendering.h"

class Creature;

struct BodyData {
    // Color components: red, green, blue, alpha
    float r, g, b, a;
    Creature* parentCreature;

    BodyData(float red, float green, float blue, float alpha)
            : r(red), g(green), b(blue), a(alpha) {}
};


b2Body* copyBody(const b2Body* sourceBody, b2World* world, float mutationRate) {
    b2BodyDef bodyDef;
    bodyDef.type = sourceBody->GetType();
    bodyDef.position = sourceBody->GetPosition();
    bodyDef.angle = sourceBody->GetAngle();
    bodyDef.linearVelocity = sourceBody->GetLinearVelocity();
    bodyDef.angularVelocity = sourceBody->GetAngularVelocity();
    bodyDef.linearDamping = sourceBody->GetLinearDamping();
    bodyDef.angularDamping = sourceBody->GetAngularDamping();
    bodyDef.allowSleep = sourceBody->IsSleepingAllowed();
    bodyDef.awake = sourceBody->IsAwake();
    bodyDef.fixedRotation = sourceBody->IsFixedRotation();
    bodyDef.bullet = sourceBody->IsBullet();
    bodyDef.active = sourceBody->IsActive();
    bodyDef.gravityScale = sourceBody->GetGravityScale();

    b2Body* newBody = world->CreateBody(&bodyDef);

    for (const b2Fixture *sourceFixture = sourceBody->GetFixtureList(); sourceFixture; sourceFixture = sourceFixture->GetNext()) {
        b2FixtureDef fixtureDef;
        fixtureDef.shape = sourceFixture->GetShape();
        fixtureDef.friction = std::min(std::max(0.0f, sourceFixture->GetFriction() * (1 + mutationRate * ((float)rand() / RAND_MAX - 0.5f))), 1.0f);
        fixtureDef.restitution = std::min(std::max(0.0f, sourceFixture->GetRestitution() * (1 + mutationRate * ((float)rand() / RAND_MAX - 0.5f))), 1.0f);
        fixtureDef.density = std::max(0.0f, sourceFixture->GetDensity() * (1 + mutationRate * ((float)rand() / RAND_MAX - 0.5f)));
        fixtureDef.isSensor = sourceFixture->IsSensor();
        fixtureDef.filter = sourceFixture->GetFilterData();

        // Check if fixture shape is a square
        if (sourceFixture->GetShape()->GetType() == b2Shape::e_polygon && ((b2PolygonShape*)sourceFixture->GetShape())->GetVertexCount() == 4) {
            auto* polygonShape = (b2PolygonShape*)sourceFixture->GetShape();

            // Mutate the size of the square
            b2Vec2 vertices[4];
            for (int i = 0; i < 4; i++) {
                b2Vec2 vertex = polygonShape->GetVertex(i);
                vertex *= (1 + mutationRate * ((float)rand() / RAND_MAX - 0.5f));
                vertices[i] = vertex;
            }
            polygonShape->Set(vertices, 4);

            fixtureDef.shape = polygonShape;
        }

        newBody->CreateFixture(&fixtureDef);
    }

    return newBody;
}


class Creature {
private:
    float health;
    float offsetX; // offset values for reproduction
    float offsetY;
    std::list<b2Body*> bodyParts; // assuming Box2D bodies make up the creature's body
public:
    Creature(std::list<b2Body *> vector) {
        health = 100.0f;
        bodyParts = std::move(vector);
        offsetX = 2.0f;
        offsetY = 2.0f;
    }
    Creature() : health(100.0f) {}

    void setHealth(float h) { health = h; }
    void addToHealth(float h) { health += h; }
    float getHealth() const { return health; }
    const std::list<b2Body*>& getBodyParts() const { return bodyParts; }
    void addBodyPart(b2Body* body) { bodyParts.push_back(body); }
    void createBodyPart(b2World* world, float x, float y, float width, float height);

    Creature* reproduce(b2World* world, float mutationRate = 0.1) const {
        std::list<b2Body*> newBodyParts;

        // Create a new Creature using the new body parts
        auto* newCreature = new Creature();

        for (const b2Body* sourceBody : getBodyParts()) {
            b2Body* newBody = copyBody(sourceBody, world, mutationRate);

            // Get the source body's user data
            BodyData* sourceUserData = static_cast<BodyData*>(sourceBody->GetUserData());

            // Create a new instance of BodyData with the correct parentCreature reference
            BodyData* newUserData = new BodyData(
                    sourceUserData->r, sourceUserData->g,
                    sourceUserData->b, sourceUserData->a
            );

            // Set the new parentCreature reference
            newUserData->parentCreature = newCreature;

            // Assign the new user data to the new body
            newBody->SetUserData(newUserData);

            newCreature->addBodyPart(newBody);
        }

        for (b2Body *newBody: newCreature->getBodyParts()) {
            b2Vec2 newPosition = newBody->GetPosition() + b2Vec2(offsetX, offsetY);
            newBody->SetTransform(newPosition, newBody->GetAngle());
        }

        // Mutate the offset values
        newCreature->offsetX *= (1 + mutationRate * ((float)rand() / RAND_MAX - 0.5f));
        newCreature->offsetY *= (1 + mutationRate * ((float)rand() / RAND_MAX - 0.5f));



        return newCreature;
    }

    // Getter and setter functions for offsetX and offsetY
    float getOffsetX() const { return offsetX; }
    void setOffsetX(float x) { offsetX = x; }
    float getOffsetY() const { return offsetY; }
    void setOffsetY(float y) { offsetY = y; }
};



#endif //LIQUIDFUN_EVO_SIM_CREATURE_H
