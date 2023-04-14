//
// Created by rwill on 4/12/2023.
//

#include "creature.h"
#include <list>
#include <Box2D/Box2D.h>

void Creature::createBodyPart(b2World* world, float x, float y, float width, float height) {
    // Create the body definition
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(x, y);

    // Create the body in the world
    b2Body* body = world->CreateBody(&bodyDef);

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
    auto* bodyData = new BodyData(0.0f, 1.0f, 0.0f, 1.0f);

    bodyData->parentCreature = this;

    // Set the user data of the b2Body
    body->SetUserData(bodyData);

    // Add the body to the creature
    addBodyPart(body);
}
