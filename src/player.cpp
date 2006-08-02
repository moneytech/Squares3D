#include <cmath>

#include "player.h"
#include "video.h"
#include "game.h"
#include "world.h"

//, const Vector& rotation
Player::Player(const string& id, const Game* game, const Vector& position, const Vector& rotation) :
    m_body(game->m_world->m_level->getBody(id)),
    m_isOnGround(true),
    m_game(game)
{
    m_body->setTransform(position, rotation);

	// set the viscous damping the the minimum
	NewtonBodySetLinearDamping(m_body->m_newtonBody, 0.0f);
    NewtonBodySetAngularDamping(m_body->m_newtonBody, Vector::Zero.v);

  	// add an up vector constraint to help in keeping the body upright
    m_upVector = NewtonConstraintCreateUpVector(m_game->m_world->m_newtonWorld, Vector::Y.v, m_body->m_newtonBody); 
    NewtonBodySetUserData(m_body->m_newtonBody, this);
    NewtonBodySetForceAndTorqueCallback(m_body->m_newtonBody, onSetForceAndTorque);

    m_body->setCollideable(this);
}

Player::~Player()
{
    NewtonDestroyJoint(m_game->m_world->m_newtonWorld, m_upVector);
}

Vector Player::getPosition()
{
    return m_body->getPosition();
}

void Player::setDirection(const Vector& direction)
{
    m_direction = direction;
}

void Player::setRotation(const Vector& rotation)
{
    m_rotation = rotation;
}

void Player::onSetForceAndTorque()
{
    //    clog << NewtonGetTimeStep(m_game->m_world->m_world) << endl;
    float timestepInv = 1.0f / 0.01f; // 0.01f == DT

    float mass;
    float Ixx, Iyy, Izz;

    // Get the mass of the object
    NewtonBodyGetMassMatrix(m_body->m_newtonBody, &mass, &Ixx, &Iyy, &Izz );

    Vector force = gravityVec * mass;
    NewtonBodyAddForce(m_body->m_newtonBody, force.v);

    Vector currentVel;
    NewtonBodyGetVelocity(m_body->m_newtonBody, currentVel.v);
      
    Vector targetVel = m_direction;

    force = (targetVel * 5.0f - currentVel ) * timestepInv * mass;
    if (!m_isOnGround && glfwGetKey(GLFW_KEY_SPACE)==GLFW_RELEASE) force.y = 0.0f;

    NewtonBodyAddForce(m_body->m_newtonBody, force.v);

    m_isOnGround = false;
  
    Vector omega;
	NewtonBodyGetOmega(m_body->m_newtonBody, omega.v);

	Vector torque = m_rotation;
    torque = (10.0f * torque - omega) * timestepInv * Iyy;
	NewtonBodySetTorque (m_body->m_newtonBody, torque.v);
}

void Player::onSetForceAndTorque(const NewtonBody* body)
{
    Player* self = static_cast<Player*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}

void Player::onCollision(const NewtonMaterial* material, const NewtonContact* contact)
{
   Vector pos, nor;

   NewtonMaterialGetContactPositionAndNormal(material, pos.v, nor.v);

   // Determine if this contact is on the ground
   float angle = Vector::Y % nor;
   m_isOnGround = (angle > 0.0f);

   NewtonMaterialSetContactElasticity( material, 0.0f );

   NewtonMaterialSetContactFrictionState( material, 0, 0 );
   NewtonMaterialSetContactFrictionState( material, 0, 1 );

   NewtonMaterialSetContactSoftness( material, 0 );

}
