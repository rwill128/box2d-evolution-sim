#include <Box2D/Box2D.h>
#include <Box2D/Particle/b2ParticleSystem.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "creature.h"
#include "rendering.h"
#include <random>
#include <chrono>


static const float WORLD_SIZE = 100.0f;

b2Body *createWorldBoundaries(b2World &world, float squareWidth);

void clampCreaturePositions(std::list<Creature *> creatureList, float minX, float maxX, float minY, float maxY) {
    for (Creature *creature: creatureList) {
        for (b2Body *bodyPart: creature->getBodyParts()) {
            b2Vec2 position = bodyPart->GetPosition();

            float clampedX = std::max(minX, std::min(position.x, maxX));
            float clampedY = std::max(minY, std::min(position.y, maxY));

            if (position.x != clampedX || position.y != clampedY) {
                bodyPart->SetTransform(b2Vec2(clampedX, clampedY), bodyPart->GetAngle());
            }
        }
    }
}


int main(int argc, char **argv) {


    // Create a world with gravity
    b2Vec2 gravity(0.0f, -1.0f);
    b2World world(gravity);

    createWorldBoundaries(world, WORLD_SIZE);

//    WaterContactListener myContactListener;
//    world.SetContactListener(&myContactListener);

    // Create a dynamic body
    auto *creature1 = new Creature("B(2,3;D;45)F(P[(0.0,0.0),(1.0,0),(1,1),(0,1)];0.5;0.5;0.5)", &world);
    auto *creature2 = new Creature("B(2,3;D;45)F(P[(0,0),(1,0),(1,1),(0,1)];1;1;1)", &world);

    std::list<Creature *> creatureList;
    creatureList.push_back(creature1);
    creatureList.push_back(creature2);

    // Create a particle system
    b2ParticleSystemDef particleSystemDef;
    particleSystemDef.radius = 0.1f;
//    particleSystemDef.gravityScale = 0.01f;
//    particleSystemDef.powderStrength = 3.0f;
    particleSystemDef.surfaceTensionNormalStrength = 1.0f;
    particleSystemDef.surfaceTensionPressureStrength = 1.0f;
    particleSystemDef.staticPressureStrength = 5.0f;
    b2ParticleSystem *particleSystem = world.CreateParticleSystem(&particleSystemDef);

    // Create a particle group
    b2PolygonShape airParticlesShape;
    airParticlesShape.SetAsBox(4, 4);

    b2ParticleGroupDef particleGroupDef;
    particleGroupDef.shape = &airParticlesShape;
    particleGroupDef.flags = b2_waterParticle;
//    particleGroupDef.flags = b2_powderParticle;
    particleGroupDef.position.Set(10.0f, 4.0f);
    particleSystem->CreateParticleGroup(particleGroupDef);

    float32 timeStep = 1.0f / 60.0f;
    int32 velocityIterations = 6;
    int32 positionIterations = 2;
    int32 particleIterations = 1;

    // Initialize GLFW and create a window
    GLFWwindow *window = initGLFW();

    // Run the physics simulation and render the scene
    while (!glfwWindowShouldClose(window)) {

        float minX = 0.5f;
        float maxX = WORLD_SIZE - 0.5f;
        float minY = 0.5f;
        float maxY = WORLD_SIZE - 0.5f;

        clampCreaturePositions(creatureList, minX, maxX, minY, maxY);

        // Define the minimum number of particles
        const int32 minParticleCount = 600;

        // Check the number of particles in the particleSystem during the main loop
        int32 currentParticleCount = particleSystem->GetParticleCount();

        // Ensure the particleSystem has at least the minimum number of particles
        if (currentParticleCount < minParticleCount) {
            int32 particlesToCreate = minParticleCount - currentParticleCount;

            // Create new particles to reach the minimum count
            // You'll need to define the particle properties using a b2ParticleDef
            b2ParticleDef particleDef;
            particleDef.flags = particleGroupDef.flags;
            particleDef.position = b2Vec2(WORLD_SIZE * ((float) rand() / RAND_MAX),
                                          WORLD_SIZE * ((float) rand() / RAND_MAX));
            particleDef.color = particleGroupDef.color;
            particleDef.lifetime = particleGroupDef.lifetime;
            particleDef.userData = particleGroupDef.userData;

            // Create the particles
            for (int32 i = 0; i < particlesToCreate; ++i) {
                particleSystem->CreateParticle(particleDef);
            }
        }


        // Get the number of body contacts
        int32 bodyContactCount = particleSystem->GetBodyContactCount();

        // Get the array of body contacts
        const b2ParticleBodyContact *bodyContacts = particleSystem->GetBodyContacts();

        // Iterate over the body contacts
        for (int32 i = 0; i < bodyContactCount; ++i) {
            // Access the i-th body contact
            const b2ParticleBodyContact &contact = bodyContacts[i];

            // Get the indices of the particle and body involved in the contact
            int32 particleIndex = contact.index;
            b2Body *contactedBody = contact.body;

            if (contactedBody->GetType() == b2_dynamicBody) {
                // Perform any desired operations using the contact information

//                particleSystem->DestroyParticle(particleIndex);
            }
        }


        // Step the world
        world.Step(timeStep, velocityIterations, positionIterations, particleIterations);

        std::list<Creature *> newCreatureList;

        // Process creature growth
        for (Creature *creatureToCheck: creatureList) {

            for (b2Body* movingBody : creatureToCheck->getBodyParts()) {

                // Define the random number generator with a seed based on current time
                std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

                // Define the uniform distribution for angles between 0 and 2π
                std::uniform_real_distribution<float> angle_distribution(0.0f, 2.0f * b2_pi);

                // Generate a random angle
                float impulse_orientation = angle_distribution(rng);
                // Apply the impulse to the body at the specified location and orientation
                float impulse_magnitude = 1;  // The magnitude of the impulse
                b2Vec2 impulse_vector = b2Vec2(impulse_magnitude * cos(impulse_orientation),
                                               impulse_magnitude * sin(impulse_orientation));
                movingBody->ApplyForceToCenter(impulse_vector, true);

            }

            creatureToCheck->addToHealth(-0.02f);

            // Reproduce successful creatures
            if (creatureToCheck->getHealth() > 200.0f) {

                creatureToCheck->addToHealth(-100.0f);

//                Creature *newCreature = creatureToCheck->reproduce(&world);


                // Add the new creature to the creatureList
//                newCreatureList.push_back(newCreature);
            }

        }

        creatureList.splice(creatureList.end(), newCreatureList);

        // Draw the scene
        drawScene(creatureList, particleSystem, WORLD_SIZE);

        for (auto it = creatureList.begin(); it != creatureList.end(); /* no increment here */) {
            Creature *creatureToCheck = *it;
            // Remove dead creatures
            // Check if the creature is dead
            if (creatureToCheck->getHealth() < 1.0f) {
                for (b2Body *bodyPart: creatureToCheck->getBodyParts()) {
                    // Remove all fixtures attached to the body
                    b2Fixture *fixture = bodyPart->GetFixtureList();
                    while (fixture != NULL) {
                        b2Fixture *next = fixture->GetNext();
                        bodyPart->DestroyFixture(fixture);
                        fixture = next;
                    }
                    // Delete the body from the world
                    world.DestroyBody(bodyPart);
                }

                // Delete the creature
                delete creatureToCheck;

                // Erase the dead creature from the list and update the iterator
                it = creatureList.erase(it);
            } else {
                // Move to the next creature if it's not dead
                ++it;
            }
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Destroy world before closing application.
    for (Creature *deleteCreature: creatureList) {
        for (b2Body *deleteBody: deleteCreature->getBodyParts()) {

            b2Fixture *fixture = deleteBody->GetFixtureList();
            while (fixture != NULL) {
                b2Fixture *next = fixture->GetNext();
                deleteBody->DestroyFixture(fixture);
                fixture = next;
            }

            world.DestroyBody(deleteBody);
        }
    }

    creatureList.clear();

    cleanUpScene();
    // Clean up GLFW
    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}


b2Body *createWorldBoundaries(b2World &world, float squareWidth) {

    b2BodyDef squareBodyDef;
    squareBodyDef.type = b2_staticBody;
    squareBodyDef.position.Set(0, 0); // You can adjust the position if needed
    b2Body* squareBody = world.CreateBody(&squareBodyDef);

    b2EdgeShape topEdge, bottomEdge, leftEdge, rightEdge;

    // Set the vertices of the edge shapes
    topEdge.Set(b2Vec2(0, squareWidth), b2Vec2(squareWidth, squareWidth));
    bottomEdge.Set(b2Vec2(0, 0), b2Vec2(squareWidth, 0));
    leftEdge.Set(b2Vec2(0, 0), b2Vec2(0, squareWidth));
    rightEdge.Set(b2Vec2(squareWidth, 0), b2Vec2(squareWidth, squareWidth));

    b2FixtureDef squareFixtureDef;
    squareFixtureDef.density = 0; // Static bodies don't need density
    squareFixtureDef.restitution = 0.5f; // Adjust the restitution (bounciness) if needed
    squareFixtureDef.friction = 0.5f; // Adjust the friction if needed

    // Attach the edge shapes to the square body
    squareFixtureDef.shape = &topEdge;
    squareBody->CreateFixture(&squareFixtureDef);

    squareFixtureDef.shape = &bottomEdge;
    squareBody->CreateFixture(&squareFixtureDef);

    squareFixtureDef.shape = &leftEdge;
    squareBody->CreateFixture(&squareFixtureDef);

    squareFixtureDef.shape = &rightEdge;
    squareBody->CreateFixture(&squareFixtureDef);

    return squareBody;
}

