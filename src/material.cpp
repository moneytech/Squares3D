#include "material.h"
#include "shader.h"
#include "xml.h"
#include "video.h"

Material::Material(const XMLnode& node, Video* video) :
    m_id(),
    m_shader(NULL),
    m_cAmbient(0.2f, 0.2f, 0.2f),
    m_cSpecular(0.0f, 0.0f, 0.0f),
    m_cEmission(0.0f, 0.0f, 0.0f),
    m_cShine(0.0f),
    m_texture(0),
    m_textureBump(0)
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
                    m_texture = video->loadTexture(node.value);
                }
                else if (node.name == "bump_path")
                {
                    m_textureBump = video->loadTexture(node.value);
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
        else if (node.name == "shader")
        {
            std::string name = getAttribute(node, "name");
            m_shader = video->loadShader(name + ".vp", name + ".fp");
        }
        else
        {
            throw Exception("Invalid material, unknown node - " + node.name);
        }
    }
}

void Material::enable(const Video* video) const
{
    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, m_cAmbient.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_cSpecular.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_cEmission.v);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_cShine);

    if (video->m_haveShaders && (m_shader != NULL))
    {
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureBump);
        video->begin(m_shader);
    }
}

void Material::disable(const Video* video) const
{
    if (video->m_haveShaders && (m_shader != NULL))
    {
        video->end(m_shader);
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
    }

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);
}