#ifndef __MENU__H__
#define __MENU__H__

#include "common.h"
#include "state.h"
#include "vmath.h"

class Music;
class Game;
class Font;
struct Face;
class Texture;
class Menu;
class Entry;
class Submenu;
class User;

typedef map<string, Submenu*> Submenus;

typedef vector<Entry*> Entries;

class Submenu
{
public:

    Entries m_entries;
    size_t  m_activeEntry;
    float   m_height;
    Vector  m_centerPos;

    Submenu(const Font* font, const Font* fontBig) : m_activeEntry(0), m_title(L""), m_font(font), m_fontBig(fontBig), m_height(0) {}
    ~Submenu();

    void addEntry(Entry* entry);
    void center(const Vector& centerPos);
    void render() const;
    void control(int key);
    void setTitle(const wstring& title, const Vector& position);
    void activateNextEntry(bool moveDown);

    Vector m_upper;
    Vector m_lower;

    wstring m_title;
    Vector m_titlePos;

private:
    const Font* m_font;
    const Font* m_fontBig;

    Vector m_previousMousePos;
};

class Menu : public State
{
public:
    Menu(string& userName);
    ~Menu();

    void control();
    void update(float delta) {} 
    void updateStep(float delta) {} 
    void prepare() {}
    void render() const;
    void loadMenu(string& userName);
    void setState(State::Type state);
    void setSubmenu(const string& submenuToSwitchTo);
    State::Type progress();

    Music*      m_music;
    const Font* m_font;
    const Font* m_fontBig;
    Submenu*    m_currentSubmenu;

    Submenus     m_submenus;
private:
    State::Type  m_state;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;

};



#endif
