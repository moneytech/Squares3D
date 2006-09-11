#include <GL/glfw.h>
#include <GL/glext.h>

#include "font.h"
#include "file.h"
#include "vmath.h"

static const Vector ShadowColor(0.1f, 0.1f, 0.1f);

typedef map<string, const Font*> FontMap;

FontMap fonts;

const Font* Font::get(const string& name)
{
    FontMap::const_iterator iter = fonts.find(name);
    if (iter != fonts.end())
    {
        return iter->second;
    }
    return fonts.insert(make_pair(name, new Font(name))).first->second;
}

void Font::unload()
{
    for each_(FontMap, fonts, iter)
    {
        delete iter->second;
    }
    fonts.clear();
}

struct BFFheader
{
    unsigned short id;
    unsigned int width;
    unsigned int height;
    unsigned int cell_width;
    unsigned int cell_height;
    unsigned char bpp;
    unsigned char base;
    unsigned char char_width[256];
};

Font::Font(const string& filename)
{
    File::Reader font("/data/font/" + filename + ".bff");
    if (!font.is_open())
    {
        throw Exception("Font file '" + filename + ".bff' not found");
    }
    char buf[20+256];
    
    if (font.read(buf, sizeof(buf)) != sizeof(buf))
    {
        throw Exception("Invalid font file");
    }
    
    BFFheader header;
    header.id =          *reinterpret_cast<unsigned short*>(&buf[0]);
    header.width =       *reinterpret_cast<unsigned int*>(&buf[2]);
    header.height =      *reinterpret_cast<unsigned int*>(&buf[6]);
    header.cell_width =  *reinterpret_cast<unsigned int*>(&buf[10]);
    header.cell_height = *reinterpret_cast<unsigned int*>(&buf[14]);
    header.bpp =         *reinterpret_cast<unsigned char*>(&buf[18]);
    header.base =        *reinterpret_cast<unsigned char*>(&buf[19]);
    std::copy(buf+20, buf+20+256, header.char_width);

    int format;

    if (header.id != 0xF2BF && header.bpp!=8 && header.bpp!=24 && header.bpp!=32)
    {
        throw Exception("Invalid font file header");
    }

    switch (header.bpp)
    {
    case 8: format = GL_ALPHA; break;
    case 24: format = GL_RGB; break;
    case 32: format = GL_RGBA; break;
    }

    vector<char> data(header.width * header.height * header.bpp/8);
    if (font.read(&data[0], data.size()) != data.size())
    {
        throw Exception("Invalid font file data");
    }
    font.close();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, format, header.width, header.height, 0, format, GL_UNSIGNED_BYTE, &data[0]);

    m_listbase = glGenLists(256);

    unsigned int maxchar = header.base + (header.width * header.height) / (header.cell_width * header.cell_height);
    int row_pitch = header.width / header.cell_width;

    float col_factor = static_cast<float>(header.cell_width) / header.width;
    float row_factor = static_cast<float>(header.cell_height) / header.height;

    float w = static_cast<float>(header.cell_width);
    float h = static_cast<float>(header.cell_height);

    for (unsigned int ch=0; ch<256; ch++)
    {
        glNewList(m_listbase + ch, GL_COMPILE);

        if (ch>=header.base && ch<=maxchar)
        {
            int idx = ch-header.base;
            int row = idx / row_pitch;
            int col = idx - row*row_pitch;

            float u1 = col * col_factor;
            float v1 = row * row_factor;
            float u2 = u1 + col_factor;
            float v2 = v1 + row_factor;

            glPushMatrix();
            glBegin(GL_QUADS);
                glTexCoord2f(u1, v1); glVertex2f(0, h);
                glTexCoord2f(u1, v2); glVertex2f(0, 0);
                glTexCoord2f(u2, v2); glVertex2f(w, 0);
                glTexCoord2f(u2, v1); glVertex2f(w, h);
            glEnd();
            glPopMatrix();
      
            glTranslatef(header.char_width[ch], 0.0, 0.0);
        }

        glEndList();
    }

    m_bpp = header.bpp;
    m_height = header.cell_height;
    for (int i=0; i<256; i++)
    {
        m_widths[i] = header.char_width[i];
    }
}

Font::~Font()
{
    glDeleteTextures(1, &m_texture);
    glDeleteLists(m_listbase, 256);
}

void Font::begin(bool shadowed, float shadowWidth) const
{
    glPushAttrib(
        GL_COLOR_BUFFER_BIT |
        GL_CURRENT_BIT |
        GL_DEPTH_BUFFER_BIT |
        GL_ENABLE_BIT |
        GL_LIGHTING_BIT |
        GL_LIST_BIT |
        GL_TRANSFORM_BIT |
        GL_POLYGON_BIT);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(viewport[0], viewport[2], viewport[1], viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    if (m_bpp != 24)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glListBase(m_listbase);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    m_shadowed = shadowed;
    m_shadowWidth = shadowWidth;
}

void Font::renderPlain(const string& text) const
{
    glPushMatrix();
    glCallLists(static_cast<unsigned int>(text.size()), GL_UNSIGNED_BYTE, text.c_str());
    glPopMatrix();
}

void Font::render(const string& text, AlignType align) const
{
    size_t begin = 0;
    size_t pos = 0;
    float nextLine = 0; 
    do
    {
        string line;
        pos = text.find('\n', begin + 1);
        if (pos != string::npos)
        {
            line = text.substr(begin, pos - begin);
            begin = pos;
        }
        else
        {
            line = text.substr(begin, text.length());
        }

        IntPair size = getSize(line);

        glPushMatrix();

        switch (align)
        {
        case Align_Left:
            break;
        case Align_Center:
            glTranslatef(static_cast<float>(-size.first/2), 0.0f, 0.0f);
            break;
        case Align_Right:
            glTranslatef(static_cast<float>(-size.first), 0.0f, 0.0f);
            break;
        default:
            assert(false);
        };

        glTranslatef(0.0f, nextLine, 0.0f);

        if (m_shadowed)
        {
            glPushAttrib(GL_CURRENT_BIT);
            glPushMatrix();
                glColor3fv(ShadowColor.v);
                glTranslatef(m_shadowWidth, - m_shadowWidth, 0.0f);
                renderPlain(line);
            glPopMatrix();
            glPopAttrib();
        }

        renderPlain(line);
        nextLine -= static_cast<float>(size.second);

        glPopMatrix();
    }
    while (pos != string::npos);
}

void Font::end() const
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
    glDepthMask(GL_TRUE);
}

IntPair Font::getSize(const string& text) const
{
    IntPair result(0, m_height);
    for (size_t i=0; i<text.size(); i++)
    {
        result.first += m_widths[text[i]];
    }
    return result;
}

