#include "fence.h"
#include "common.h"
#include "level.h"
#include "collision.h"
#include "vmath.h"
#include "body.h"

void makeFence(Level* level, const NewtonWorld* newtonWorld)
{
    Collision* fenceCollision = level->getCollision("fence");

    Collision* heightMap = level->getCollision("level");

    const float fenceWidth = 0.3f;
    const float fenceHeight = 1.0f;
    const float fenceSpacing = fenceWidth + fenceWidth / 5;

    for (size_t i = 0; i < level->m_fences.size(); i++)
    {
        Vector startPoint = level->m_fences[i].first;
        Vector endPoint = level->m_fences[i].second;
        Vector delta(endPoint - startPoint);

        Vector direction = delta;
        Vector rotation(0, - direction.getRotationY(), 0);
        
        float howMany = delta.magnitude() / fenceSpacing;
        delta /= howMany;
        for (int j = 0; j < howMany; j++)
        {
            string bodyID = "fence" + cast<string>(i) + "_" + cast<string>(j);
            Body* body = new Body(bodyID, level, fenceCollision);
            Vector position = Vector(startPoint + delta * static_cast<float>(j));
            position.y = fenceHeight / 2 + heightMap->getHeight(position.x, position.z) + 0.05f;
            body->setTransform(position, rotation);
            NewtonWorldFreezeBody(newtonWorld, body->m_newtonBody);
            NewtonBodySetAutoFreeze(body->m_newtonBody, 1);
            level->m_bodies[bodyID] = body;
        }
    }
}

