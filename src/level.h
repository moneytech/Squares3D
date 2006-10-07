#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class XMLnode;
class Material;    
class Properties;
class Collision;
class Body;
class Character;
class Profile;
struct Face;

typedef map<string, Material*>       MaterialsMap;
typedef map<string, Collision*>      CollisionsMap;
typedef map<string, Body*>           BodiesMap;
typedef map<string, Character*>      CharactersMap;
typedef map<string, Profile*>        ProfilesMap;
typedef set<pair<Face*, Material*> > FaceSet;

class Level : NoCopy
{
public:
    Level();
    ~Level();
    void  load(const string& levelFile, StringSet& loaded = StringSet());
    void  render() const;
    void  prepare();
    Body* getBody(const string& id) const;
    Collision* getCollision(const string& id) const;
    Character* getCharacter(const string& type) const;

    BodiesMap     m_bodies;
    CollisionsMap m_collisions;
    FaceSet       m_faces;
    MaterialsMap  m_materials;
    CharactersMap m_characters;
    ProfilesMap   m_cpuProfiles;
    Properties*   m_properties;
};


#endif
