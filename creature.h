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
#include <random>

class Creature {
private:
    float health;
    float mass;
    float energyToContributeToChildren;
    float inheritedMutationRate;
    int numberOfOffspring;

    b2World *m_world;
    std::vector<b2Body *> m_bodies;

    b2BodyDef copyBodyDef(b2Body* originalBody) {
        b2BodyDef bodyDef;

        bodyDef.type = originalBody->GetType();
        bodyDef.position = originalBody->GetPosition();
        bodyDef.angle = originalBody->GetAngle();
        bodyDef.linearDamping = originalBody->GetLinearDamping();
        bodyDef.angularDamping = originalBody->GetAngularDamping();
        bodyDef.fixedRotation = originalBody->IsFixedRotation();
        bodyDef.bullet = originalBody->IsBullet();
        bodyDef.gravityScale = originalBody->GetGravityScale();

        return bodyDef;
    }


    b2Body* copyBodyWithMutation(b2Body* originalBody, b2World* world, float mutationRate) {
        // Copy the body definition from the original body
        b2BodyDef bodyDef = copyBodyDef(originalBody);

        // Create a random number generator
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> mutationDist(-mutationRate, mutationRate);

        // Mutate the position
        float dx = mutationDist(gen);
        float dy = mutationDist(gen);
        bodyDef.position.x += dx;
        bodyDef.position.y += dy;

        // Mutate the angle
        float angleMutation = mutationDist(gen);
        bodyDef.angle += angleMutation;

        // Create a new body using the mutated body definition
        b2Body* newBody = world->CreateBody(&bodyDef);

        // Copy the fixtures from the original body to the new body
        for (b2Fixture* originalFixture = originalBody->GetFixtureList(); originalFixture; originalFixture = originalFixture->GetNext()) {
            b2FixtureDef fixtureDef;
            fixtureDef.shape = originalFixture->GetShape();

            // Mutate the density, friction, and restitution
            float mutatedDensity = std::max(0.0f, originalFixture->GetDensity() + (float) mutationDist(gen));
            float mutatedFriction = std::clamp(originalFixture->GetFriction() + (float) mutationDist(gen), 0.0f, 1.0f);
            float mutatedRestitution = std::clamp(originalFixture->GetRestitution() + (float) mutationDist(gen), 0.0f, 1.0f);

            fixtureDef.density = mutatedDensity;
            fixtureDef.friction = mutatedFriction;
            fixtureDef.restitution = mutatedRestitution;

            fixtureDef.isSensor = originalFixture->IsSensor();
            fixtureDef.filter = originalFixture->GetFilterData();

            newBody->CreateFixture(&fixtureDef);
        }

        return newBody;
    }

    Creature(Creature* parent, b2World *world, float energyContribution, float mutationRate) : m_world(world) {
        health = energyContribution;

        // Mutate energyToContributeToChildren and numberOfOffspring
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> mutationDist(-mutationRate, mutationRate);
        float energyToContributeToChildrenMutation = 1.0f + mutationDist(gen);
        float numberOfOffspringMutation = 1.0f + mutationDist(gen);

        energyToContributeToChildren = parent->energyToContributeToChildren * energyToContributeToChildrenMutation;
        numberOfOffspring = std::round(parent->numberOfOffspring * numberOfOffspringMutation);

        // Ensure numberOfOffspring is at least 1
        numberOfOffspring = std::max(1, numberOfOffspring);
        energyToContributeToChildren = std::max(1.0f, energyToContributeToChildren);

        mass = 0.0f;

        // Create mutated copies of the original bodies from the parent and add them to m_bodies
        for (b2Body *originalBody : parent->m_bodies) {
            b2Body *mutatedBody = copyBodyWithMutation(originalBody, world, mutationRate);
            m_bodies.push_back(mutatedBody);
            mass += mutatedBody->GetMass();
        }
    }


public:
    Creature(const std::vector<b2Body *> &originalBodies, b2World *world, float startingHealth, float energyToContribute, int numberOfOffSpringP) : m_world(world), energyToContributeToChildren(energyToContribute) {
        health = startingHealth;
        mass = 0.0f;
        numberOfOffspring = numberOfOffSpringP;

        // Create mutated copies of the original bodies and add them to m_bodies
        for (b2Body *originalBody : originalBodies) {
            m_bodies.push_back(originalBody);
            mass += originalBody->GetMass();
        }
    }

    std::vector<Creature *> reproduce(b2World *world) {
        std::vector<Creature *> offspring;

        // Calculate the energy contributed to each child
        float energyPerChild = energyToContributeToChildren / numberOfOffspring;
        this->health -= energyToContributeToChildren;

        // Create new Creatures with mutated copies of the body parts
        for (int i = 0; i < numberOfOffspring; ++i) {
            Creature child(this, world, energyPerChild, inheritedMutationRate);
            offspring.push_back(&child);
        }

        return offspring;
    }

    void addToHealth(float amount) {
        health += amount;
    }

    float getHealth() {
        return health;
    }

    std::vector<b2Body *> getBodyParts() {
        return m_bodies;
    }


    // Other methods, like drawing, updating physics, etc.
};
;


#endif //LIQUIDFUN_EVO_SIM_CREATURE_H
