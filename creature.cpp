//
// Created by rwill on 4/12/2023.
//

#include "creature.h"
#include <list>
#include <Box2D/Box2D.h>



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

Creature* Creature::reproduce(b2World* world, float mutationRate) const  {
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
