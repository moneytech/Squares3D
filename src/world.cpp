#include "player.h"
#include "camera.h"
#include "world.h"
#include "video.h"
#include "audio.h"
#include "game.h"
#include "player_local.h"
#include "input.h"
#include "level.h"
#include "music.h"
#include "file.h"

/*
struct ContactBodies
{
    const NewtonBody* body1;
    const NewtonBody* body2;
};

// this callback is called when the aabb box with player overlaps floor
int  PlayerContactBegin(const NewtonMaterial* material, const NewtonBody* body1, const NewtonBody* body2)
{
	ContactBodies* contactBodies = static_cast<ContactBodies*>(NewtonMaterialGetMaterialPairUserData(material));
    contactBodies->body1 = body1;
    contactBodies->body2 = body2;

    return 1;
}

static int PlayerContactProcess(const NewtonMaterial* material, const NewtonContact* contact)
{
	ContactBodies* contactBodies = static_cast<ContactBodies*>(NewtonMaterialGetMaterialPairUserData(material));
    
	Player* player[2];

    player[0] = static_cast<Player*>(NewtonBodyGetUserData(contactBodies->body2));
    player[1] = static_cast<Player*>(NewtonBodyGetUserData(contactBodies->body1));
    
    for (int i=0; i<2; i++)
    {
        if (player[i] && player[i]->getType()==Body::PlayerBody)
        {
            player[i]->onCollision(material, contact);
        }
    }

    return 1;
}

ContactBodies contactBodies;
*/

Music* music;

World::World(Game* game) : 
    Renderable(game),
    m_camera(new Camera(game))
{
    m_world = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(m_world, static_cast<void*>(this));
    NewtonSetSolverModel(m_world, 10);
    NewtonSetFrictionModel(m_world, 1);
    
    music = m_game->m_audio->loadMusic("music.ogg");
    //music->play();
}

void World::init()
{
	int charID = NewtonMaterialCreateGroupID(m_world);

	//NewtonMaterialSetDefaultElasticity(m_world, floorID, charID, 0.4f);
	//NewtonMaterialSetDefaultFriction(m_world, floorID, charID, 0.4f, 0.4f);

	//NewtonMaterialSetCollisionCallback(m_world, floorID, charID, 
 //       static_cast<void*>(&contactBodies), 
 //       PlayerContactBegin, PlayerContactProcess, NULL); 

 //   NewtonBodySetMaterialGroupID(floorBody, floorID);

    m_level.reset(new LevelObjects::Level(m_game));
    m_level->load("/data/level.xml");

    m_localPlayers.insert(new LocalPlayer("playerDura", m_game, Vector(1.0f, 2.0f, 0.0f), Vector(0.0f, 0.0f, 0.0f)));
    m_localPlayers.insert(new LocalPlayer("player", m_game, Vector(4.0f, 2.0f, 2.0f), Vector(180.0f, 0.0f, 0.0f)));
}

World::~World()
{
    music->stop();
    //delete music;

    for each_const(set<Player*>, m_localPlayers, player)
    {
        delete *player;
    }

    delete m_level.release();

    NewtonMaterialDestroyAllGroupID(m_world);
    NewtonDestroyAllBodies(m_world);
    NewtonDestroy(m_world);
}

void World::control(const Input* input)
{
    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE)
    {
        m_camera->control(input);
        for each_const(set<Player*>, m_localPlayers, player)
        {
            (*player)->control(input);
        }
    }

    // other objects go here
    // ...
}

void World::update(float delta)
{
    NewtonUpdate(m_world, delta);
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render(const Video* video) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera->render(video);

    m_level->render(video);

    video->renderAxes();
    glfwSwapBuffers();
}
