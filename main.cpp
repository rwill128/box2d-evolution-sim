#include <Box2D/Box2D.h>
#include <Box2D/Particle/b2ParticleSystem.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "creature.h"
#include "rendering.h"
#include <random>
#include <chrono>


static const float WORLD_SIZE = 20.0f;

void createWorldBoundaries(b2World *world, float worldWidth, float worldHeight);

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


#include <algorithm>
#include <iterator>
#include <stdexcept>

std::string perform_translocation_deletion_and_substitution(const std::string &geneticCode, float mutationRate) {
    std::istringstream input(geneticCode);
    std::ostringstream output;
    std::string buffer;
    std::vector<std::string> chunks;

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> probDist(0, 1);
    std::uniform_int_distribution<> operationDist(1, 3); // 1 for deletion, 2 for duplication, 3 for translocation

    // Read the immutable part (body definition) and write it to the output
    std::getline(input, buffer, '|');
    output << buffer;

    // Split the input into chunks, delimited by '|'
    while (std::getline(input, buffer, '|')) {
        chunks.push_back(buffer);
    }

    // Perform the mutation operations
    for (size_t i = 0; i < chunks.size(); ++i) {
        if (probDist(gen) <= mutationRate) {
            int operation = operationDist(gen);
            size_t targetIndex;

            switch (operation) {
                case 1: // Deletion
                    if (chunks.size() > 1) { // Check if there's more than one chunk
                        chunks.erase(chunks.begin() + i);
                        i--; // Decrement i so that the next iteration processes the same index
                    }
                    break;

                case 2: // Duplication
                    targetIndex = (i + 1) % chunks.size();
                    chunks.insert(chunks.begin() + targetIndex, chunks[i]);
                    i++; // Increment i so that the next iteration processes the index after the inserted chunk
                    break;

                case 3: // Translocation
                    if (chunks.size() > 1) {
                        do {
                            targetIndex = std::uniform_int_distribution<>(0, chunks.size() - 1)(gen);
                        } while (targetIndex == i);

                        std::swap(chunks[i], chunks[targetIndex]);
                    }
                    break;

                default:
                    throw std::runtime_error("Unknown mutation operation");
            }
        }
    }

    // Write the mutated chunks to the output
    for (const auto &chunk: chunks) {
        output << '|' << chunk;
    }

    output << '|';

    return output.str();
}


std::string mutate(const std::string &geneticCode, float mutationRate) {

    std::string translocated_genetic_code = perform_translocation_deletion_and_substitution(geneticCode, mutationRate);

    std::istringstream input(translocated_genetic_code);
    std::ostringstream output;

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> mutationDist(-mutationRate, mutationRate);

    char ch;
    float minLimit, maxLimit;
    bool mutateNext = false;

    while (input.get(ch)) {
        if (ch == '{') {
            input >> minLimit;
            input.ignore(1); // Ignore the comma
            input >> maxLimit;
            input.ignore(1); // Ignore the closing curly brace
            mutateNext = true;

            // Output the minLimit and maxLimit along with the curly braces
            output << '{' << minLimit << ',' << maxLimit << '}';
        } else if (mutateNext && std::isdigit(ch)) {
            input.putback(ch);
            float value;
            input >> value;
            value += mutationDist(gen);
            value = std::clamp(value, minLimit, maxLimit);
            output << value;
            mutateNext = false;
        } else {
            output << ch;
        }
    }

    return output.str();
}


int main(int argc, char **argv) {


    // Create a world with gravity
    b2Vec2 gravity(0.0f, 0.0f);
    b2World world(gravity);


//    WaterContactListener myContactListener;
//    world.SetContactListener(&myContactListener);

    // Create a b2Body with a square b2Fixture
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(5.0f, 5.0f);
    b2Body *boxBody = world.CreateBody(&bodyDef);

    b2PolygonShape boxShape;
    boxShape.SetAsBox(0.5f, 0.5f); // A 1x1 square has half-width and half-height of 0.5

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    boxBody->CreateFixture(&fixtureDef);

    // Create a vector containing the created b2Body
    std::vector<b2Body *> bodies = {boxBody};

    // Create a Creature object using the public constructor
    float startingHealth = 100.0f;
    float energyToContribute = 50.0f;
    int numberOfOffspring = 2;
    Creature creature = Creature(bodies, &world, startingHealth, energyToContribute, numberOfOffspring);

    createWorldBoundaries(&world, WORLD_SIZE, WORLD_SIZE);

    std::vector<Creature *> creatureList;
    creatureList.push_back(&creature);

    // Create a particle system
    b2ParticleSystemDef particleSystemDef;
    particleSystemDef.radius = 0.1f;
    particleSystemDef.gravityScale = 0.00f;
//    particleSystemDef.powderStrength = 3.0f;
    particleSystemDef.surfaceTensionNormalStrength = 1.0f;
    particleSystemDef.surfaceTensionPressureStrength = 1.0f;
    particleSystemDef.staticPressureStrength = 5.0f;
    b2ParticleSystem *particleSystem = world.CreateParticleSystem(&particleSystemDef);

    // Create a particle group
    b2PolygonShape airParticlesShape;
    airParticlesShape.SetAsBox(2, 2);

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
            b2Body *contactedBody = contact.body;

            if (contactedBody->GetType() == b2_dynamicBody) {
                // Perform any desired operations using the contact information
                Creature *creatureA = static_cast<Creature *>(contactedBody->GetUserData());

                creatureA->addToHealth(0.1);

            }
        }


        // Step the world
        world.Step(timeStep, velocityIterations, positionIterations, particleIterations);

        std::vector<Creature *> newCreatureList;

        // Process creature growth
        for (Creature *creatureToCheck: creatureList) {

            for (b2Body *movingBody: creatureToCheck->getBodyParts()) {

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
            if (creatureToCheck->getHealth() > 1000.0f) {

                std::vector<Creature *> children = creatureToCheck->reproduce(&world);

//                 Add the new creature to the creatureList
                newCreatureList.insert(newCreatureList.end(), children.begin(), children.end());
            }

        }

        creatureList.insert(creatureList.end(), newCreatureList.begin(), newCreatureList.end());

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


#include <Box2D/Box2D.h>

void createWorldBoundaries(b2World *world, float worldWidth, float worldHeight) {
    const float boundaryThickness = 1.0f;

    // Define the shape, fixture, and body for the boundaries
    b2PolygonShape boundaryShape;
    b2FixtureDef boundaryFixtureDef;
    boundaryFixtureDef.shape = &boundaryShape;
    boundaryFixtureDef.density = 1.0f; // Static bodies don't need density
    boundaryFixtureDef.friction = 0.3f;
    boundaryFixtureDef.restitution = 0.5f;

    // Create four static bodies (left, right, top, and bottom)
    b2BodyDef boundaryBodyDef;
    boundaryBodyDef.type = b2_staticBody;

    // Left boundary
    boundaryShape.SetAsBox(boundaryThickness / 2.0f, worldHeight / 2.0f);
    boundaryBodyDef.position.Set(-boundaryThickness / 2.0f, worldHeight / 2.0f);
    b2Body *leftBoundary = world->CreateBody(&boundaryBodyDef);
    leftBoundary->CreateFixture(&boundaryFixtureDef);

    // Right boundary
    boundaryShape.SetAsBox(boundaryThickness / 2.0f, worldHeight / 2.0f);
    boundaryBodyDef.position.Set(worldWidth + boundaryThickness / 2.0f, worldHeight / 2.0f);
    b2Body *rightBoundary = world->CreateBody(&boundaryBodyDef);
    rightBoundary->CreateFixture(&boundaryFixtureDef);

    // Top boundary
    boundaryShape.SetAsBox(worldWidth / 2.0f, boundaryThickness / 2.0f);
    boundaryBodyDef.position.Set(worldWidth / 2.0f, -boundaryThickness / 2.0f);
    b2Body *topBoundary = world->CreateBody(&boundaryBodyDef);
    topBoundary->CreateFixture(&boundaryFixtureDef);

    // Bottom boundary
    boundaryShape.SetAsBox(worldWidth / 2.0f, boundaryThickness / 2.0f);
    boundaryBodyDef.position.Set(worldWidth / 2.0f, worldHeight + boundaryThickness / 2.0f);
    b2Body *bottomBoundary = world->CreateBody(&boundaryBodyDef);
    bottomBoundary->CreateFixture(&boundaryFixtureDef);
}

