#include "level.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "game.h"
#include "world.h"

string getAttribute(const XMLnode& node, const string& name)
{
    if (foundInMap(node.attributes, name))
    {
        return node.attributes.find(name)->second;
    }

    throw Exception("Missing attribute '" + name + "' in node '" + node.name + "'");
}

Vector getAttributesInVector(const XMLnode& node, const string& attributeSymbols)
{
    Vector vector;
    for (size_t i = 0; i < attributeSymbols.size(); i++)
    {
        string key(1, attributeSymbols[i]);
        vector[i] = cast<float>(getAttribute(node, key));
    }
    return vector;

}

using namespace LevelObjects;

Material::Material(const XMLnode& node, const Game* game) :
    m_id(),
    m_texPath(),
    m_cAmbient(0.2f, 0.2f, 0.2f),
    m_cDiffuse(0.8f, 0.8f, 0.8f),
    m_cSpecular(0.0f, 0.0f, 0.0f),
    m_cEmission(0.0f, 0.0f, 0.0f),
    m_cShine(0.0f),
    m_texture(0)
{
    m_id = getAttribute(node, "id");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "texture")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "path")
                {
                    m_texPath = node.value;
                }
                else
                {
                    throw Exception("Invalid texture, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "colors")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "ambient")
                {
                    m_cAmbient = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "diffuse")
                {
                    m_cDiffuse = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "specular")
                {
                    m_cSpecular = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "emission")
                {
                    m_cEmission = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "shine")
                {
                    m_cShine = cast<float>(node.value);
                }
                else
                {
                    throw Exception("Invalid color, unknown node - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid material, unknown node - " + node.name);
        }
    }
    m_texture = game->m_video->loadTexture(m_texPath);
}

void Material::render(const Video* video) const
{
    video->applyTexture(m_texture);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_cAmbient.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_cDiffuse.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_cSpecular.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_cEmission.v);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_cShine);
}

Level::Level(const Game* game) :
    m_game(game)
{
}

void Level::load(const string& levelFile)
{
    clog << "Reading level data." << endl;

    XMLnode xml;
    File::Reader in(levelFile);
    if (!in.is_open())
    {
        throw Exception("Level file '" + levelFile + "' not found");  
    }
    in >> xml;
    in.close();

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "bodies")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "body")
                {
                    m_bodies.insert(new Body(node, m_game));
                }
                else
                {
                    throw Exception("Invalid body, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "materials")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "material")
                {
                    m_materials[getAttribute(node, "id")] = new Material(node, m_game);
                }
                else if (node.name == "properties")
                {
                    
                }
                else
                {
                    throw Exception("Invalid materials, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "collisions")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "collision")
                {
                    m_collisions[getAttribute(node, "id")] = Collision::create(node, m_game);
                }
                else
                {
                    throw Exception("Invalid collisions, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "joints")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "joint")
                {

                }
                else
                {
                    throw Exception("Invalid joint, unknown node - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid level file, unknown section - " + node.name);
        }
    }
}

Level::~Level()
{
    for each_(set<Body*>, m_bodies, iter)
    {
        delete *iter;
    }
    for each_const(CollisionsMap, m_collisions, iter)
    {
        delete iter->second;
    }
    for each_(MaterialsMap, m_materials, iter)
    {
        delete iter->second;
    }
}

void Level::prepare()
{
    for each_const(set<LevelObjects::Body*>, m_bodies, iter)
    {
        (*iter)->prepare();
    }
}

void Level::render(const Video* video) const
{
    // TODO: Remove
    glEnable(GL_TEXTURE_2D);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_COLOR_MATERIAL);
    for each_const(set<Body*>, m_bodies, iter)
    {
       (*iter)->render(video, &m_materials);
    }    
    glPopAttrib();
    
    // TODO: Remove
    glDisable(GL_TEXTURE_2D);
}

Body::Body(const XMLnode& node, const Game* game):
    m_id(), 
    m_material(), 
    m_matrix(),
    m_totalMass(0.0f),
    m_totalInertia(0.0f, 0.0f, 0.0f),
    m_newtonWorld(game->m_world->m_world)
{
    m_id = getAttribute(node, "id");

    Vector position(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    string name = "material";
    if (foundInMap(node.attributes, name))
    {
        m_material = getAttribute(node, name);
    }

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "position")
        {
            position = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "rotation")
        {
            rotation = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "collision")
        { 
            string id = node.value;
            CollisionsMap::const_iterator iter = game->m_world->m_level->m_collisions.find(id);
            if (iter != game->m_world->m_level->m_collisions.end())
            {
                m_collisions.insert(iter->second);
            }
            else
            {
                throw Exception("Could not find specified collision for body '" + m_id + "'");
            }
        }
        else
        {
            throw Exception("Invalid body, unknown node - " + node.name);
        }
    }

    static int i = 0;
    i++;

    //right now we support just one collision per body
    if (m_collisions.size() == 1)
    {
        Collision* collision = *m_collisions.begin();

        m_totalMass += collision->m_mass;
        m_totalInertia += collision->m_inertia;
        
        m_newtonBody = NewtonCreateBody(game->m_world->m_world, collision->m_newtonCollision);
        NewtonBodySetUserData(m_newtonBody, static_cast<void*>(this));

        NewtonBodySetMassMatrix(m_newtonBody, m_totalMass, m_totalInertia.x, m_totalInertia.y, m_totalInertia.z);
        
        NewtonBodySetForceAndTorqueCallback(m_newtonBody, onSetForceAndTorque);

        //m_matrix = m_matrix.identity();
        NewtonSetEulerAngle(rotation.v, m_matrix.m);
        m_matrix = Matrix::translate(position) * m_matrix;
                
        NewtonBodySetMatrix(m_newtonBody, m_matrix.m);

        NewtonBodySetAutoFreeze(m_newtonBody, 0);
	    //NewtonWorldUnfreezeBody(game->m_world->m_world, m_newtonBody);


        NewtonReleaseCollision(game->m_world->m_world, collision->m_newtonCollision);

        // Set Material Id for this object
	    //NewtonBodySetMaterialGroupID(m_body, material);
    }

}

Body::~Body()
{
    for each_const(set<Collision*>, m_collisions, iter)
    {
        //NewtonReleaseCollision(m_newtonWorld, (*iter)->m_newtonCollision);
    }
}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_newtonBody, m_matrix.m);
}

void Body::onSetForceAndTorque()
{
    Vector force = gravityVec * m_totalMass;
    NewtonBodyAddForce(m_newtonBody, force.v);
}

void Body::onSetForceAndTorque(const NewtonBody* body)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}

void Body::render(const Video* video, const MaterialsMap* materials)
{
    MaterialsMap::const_iterator material = materials->find(m_material);
    if (material != materials->end())
    {
        material->second->render(video);
    }

    video->begin(m_matrix);

    for each_const(set<Collision*>, m_collisions, iter)
    {
        (*iter)->render(video, materials);
    }

    video->end();
}

Collision::Collision(const XMLnode& node, const Game* game) :
    m_inertia(), m_mass(0.0f), m_newtonWorld(game->m_world->m_world)
{
}

Collision::~Collision()
{
    //NewtonReleaseCollision(m_newtonWorld, m_newtonCollision);
}

void Collision::create(NewtonCollision* collision)
{
    m_newtonCollision = collision;
}

void Collision::create(NewtonCollision* collision, float mass, const Vector& position)
{
    m_newtonCollision = collision;
    m_mass = mass;

    // TODO: wtf???
    Vector p;
    NewtonConvexCollisionCalculateInertialMatrix(m_newtonCollision, m_inertia.v, p.v);
}

CollisionBox::CollisionBox(const XMLnode& node, const Game* game) :
    Collision(node, game),
    m_size(1.0f, 1.0f, 1.0f), 
    m_hasOffset(false)
{
    Vector offset(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    float mass = cast<float>(getAttribute(node, "mass"));

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "size")
        {
            m_size = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "offset")
        {
            offset = getAttributesInVector(node, "xyz");
            m_hasOffset = true;
        }
        else if (node.name == "rotation")
        {
            rotation = getAttributesInVector(node, "xyz");
            m_hasOffset = true;
        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    if (m_hasOffset)
    {
        NewtonSetEulerAngle(rotation.v, m_matrix.m);
        m_matrix = Matrix::translate(offset) * m_matrix;
    }

    create(
        NewtonCreateBox(game->m_world->m_world, m_size.x, m_size.y, m_size.z, (m_hasOffset ? m_matrix.m : NULL)),
        mass,
        offset);
}

void CollisionBox::render(const Video* video, const MaterialsMap* materials) const
{
    glPushMatrix();

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glScalef(m_size.x, m_size.y, m_size.z);
    video->renderCube();

    glPopMatrix();
}

Collision* Collision::create(const XMLnode& node, const Game* game)
{
    string type = getAttribute(node, "type");
    
    if (type == "box")
    {
        return new CollisionBox(node, game);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node, game);
    }
    else
    {
        throw Exception("Unknown collision type - " + type);
    }
}

CollisionTree::CollisionTree(const XMLnode& node, const Game* game) :
    Collision(node, game)
{
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "face")
        {
            m_materials.push_back(getAttribute(node, "material"));
            m_faces.push_back(Face());
            Face& face = m_faces.back();
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "vertex")
                {
                    face.vertexes.push_back(getAttributesInVector(node, "xyz"));
                    face.uv.push_back(UV(
                        cast<float>(getAttribute(node, "u")),
                        cast<float>(getAttribute(node, "v"))));
                }
                else
                { 
                    throw Exception("Invalid face, unknown node - " + node.name);
                }
            }
            Vector v0 = m_faces.back().vertexes[0];
            Vector v1 = m_faces.back().vertexes[1];
            Vector v2 = m_faces.back().vertexes[2];
            
            face.normal = (v1-v0) ^ (v2-v0);
            face.normal.norm();
        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }
    NewtonCollision* collision = NewtonCreateTreeCollision(game->m_world->m_world, NULL);
    NewtonTreeCollisionBeginBuild(collision);
    for each_const(vector<Face>, m_faces, iter)
    {
        NewtonTreeCollisionAddFace(collision, iter->vertexes.size(), iter->vertexes[0].v, sizeof(Vector), 0);
    }
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);
}

void CollisionTree::render(const Video* video, const MaterialsMap* materials) const
{
    for (size_t i = 0; i < m_faces.size(); i++)
    {
        materials->find(m_materials[i])->second->render(video);

        video->renderFace(m_faces[i]);
    }
}