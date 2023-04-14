//
// Created by rwill on 4/12/2023.
//

#ifndef LIQUIDFUN_EVO_SIM_CREATURE_H
#define LIQUIDFUN_EVO_SIM_CREATURE_H

#include <list>
#include <Box2D/Dynamics/b2Body.h>
#include <utility>
#include <vector>

class Creature {
private:
    float health;
    std::list<b2Body*> bodyParts; // assuming Box2D bodies make up the creature's body
public:
    Creature(std::list<b2Body *> vector) {
        health = 100.0f;
        bodyParts = std::move(vector);
    }
    Creature() : health(100.0f) {}

    void setHealth(float h) { health = h; }
    void addToHealth(float h) { health += h; }
    float getHealth() const { return health; }
    const std::list<b2Body*>& getBodyParts() const { return bodyParts; }
    void addBodyPart(b2Body* body) { bodyParts.push_back(body); }
    void createBodyPart(b2World* world, float x, float y, float width, float height);
};

struct BodyData {
    // Color components: red, green, blue, alpha
    float r, g, b, a;
    Creature* parentCreature;

    BodyData(float red, float green, float blue, float alpha)
            : r(red), g(green), b(blue), a(alpha) {}
};

#endif //LIQUIDFUN_EVO_SIM_CREATURE_H
