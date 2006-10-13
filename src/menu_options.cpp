#include <GL/glfw.h>

#include "menu_options.h"
#include "menu_submenu.h"
#include "menu.h"

#include "config.h"
#include "input.h"
#include "video.h"
#include "language.h"
#include "font.h"
#include "game.h"
#include "config.h"
#include "sound.h"

void OptionEntry::click(int button)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT)
        || (button == GLFW_MOUSE_BUTTON_RIGHT)
        || (button == GLFW_KEY_LEFT)
        || (button == GLFW_KEY_RIGHT)
        || (button == GLFW_KEY_ENTER))
    {
        if (m_enabled)
        {
            m_value.activateNext((button == GLFW_MOUSE_BUTTON_LEFT) 
                                  || (button == GLFW_KEY_RIGHT)
                                  || (button == GLFW_KEY_ENTER));
            
            m_menu->m_sound->play(m_menu->m_soundChange);
        }
    }
}

void OptionEntry::reset()
{
    // Sync option values to values in Config object

    if (m_value.m_id == "environment_details")
    {
        int shMaps = Config::instance->m_video.shadowmap_size;
        int grDens = Config::instance->m_video.grass_density;
        int terDet = Config::instance->m_video.terrain_detail;
        
        for (int i = 0; i < 4; i++)
        {
            if (((!Video::instance->m_haveShadows) || (shMaps == i)) && (grDens == i) && (terDet == i))
            {
                m_value.m_current = i;
                break;
            }
            m_value.m_current = i;
        }
    }
    else if (m_value.m_id == "resolution")
    {
        const IntPairVector resolutions = Video::instance->getModes();
        for (size_t i = 0; i < resolutions.size(); i++)
        {
            if (Video::instance->getResolution() == resolutions[i])
            {
                m_value.m_current = i;
                break;
            }
        }
    }
    else if (m_value.m_id == "fullscreen")
    {
        m_value.m_current = Config::instance->m_video.fullscreen ? 1 : 0;
    }
    else if (m_value.m_id == "vsync")
    {
        m_value.m_current = Config::instance->m_video.vsync ? 1 : 0;
    }
    else if (m_value.m_id == "fsaa_samples")
    {
        m_value.m_current = Config::instance->m_video.samples / 2;
    }
    else if (m_value.m_id == "use_shaders")
    {
        if (!Video::instance->m_haveShaders)
        {
            m_enabled = false;
        }
        m_value.m_current = Config::instance->m_video.use_shaders ? 1 : 0;
    }
    else if (m_value.m_id == "shadow_type")
    {
        if (!Video::instance->m_haveShadows)
        {
            m_enabled = false;
        }
        m_value.m_current = Config::instance->m_video.shadow_type ? 1 : 0;
    }
    else if (m_value.m_id == "shadowmap_size")
    {
        if (!Video::instance->m_haveShadows)
        {
            m_enabled = false;
            m_value.m_current = 3; //not supported
        }
        else
        {
            m_value.m_current = Config::instance->m_video.shadowmap_size;
        }
    }
    else if (m_value.m_id == "show_fps")
    {
        m_value.m_current = Config::instance->m_video.show_fps ? 1 : 0;
    }
    else if (m_value.m_id == "grass_density")
    {
        m_value.m_current = Config::instance->m_video.grass_density;
    }
    else if (m_value.m_id == "terrain_detail")
    {
        m_value.m_current = Config::instance->m_video.terrain_detail;
    }
    else if (m_value.m_id == "audio")
    {
        m_value.m_current = Config::instance->m_audio.enabled ? 1 : 0;
    }
    else if (m_value.m_id == "language")
    {
        StringVector languages = Language::instance->getAvailable();
        for (size_t i = 0; i < languages.size(); i++)
        {
            if (languages[i] == Config::instance->m_misc.language)
            {
                m_value.m_current = i;
                break;
            }
        }
    }
    else if (m_value.m_id == "mouse_sensitivity")
    {
        m_value.m_current = static_cast<int>(Config::instance->m_misc.mouse_sensitivity*2) - 2;
    }
    else if (m_value.m_id == "music_vol")
    {
        m_value.m_current = Config::instance->m_audio.music_vol;
    }
    else if (m_value.m_id == "sound_vol")
    {
        m_value.m_current = Config::instance->m_audio.sound_vol;
    }
}

void OptionEntry::render(const Font* font) const
{
    glPushMatrix();
    font->render(m_string + L":", Font::Align_Right);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(static_cast<float>(font->getWidth(L"  ")), 0.0f, 0.0f);
    font->render(getValue(), Font::Align_Left);
    glPopMatrix();
}

void ApplyOptionsEntry::click(int button)
{
    if ((button != GLFW_MOUSE_BUTTON_LEFT) && (button != GLFW_KEY_ENTER) && (button != GLFW_KEY_KP_ENTER))
    {
        return;
    }

    //save config settings
    for each_const(Entries, m_menu->m_currentSubmenu->m_entries, iter)
    {
        string id = (*iter)->getValueID();

        if (id == "environment_details")
        {
            if ((*iter)->getCurrentValueIdx() != 3)
            {
                if (Video::instance->m_haveShadows)
                {
                    Config::instance->m_video.shadowmap_size = (*iter)->getCurrentValueIdx();
                }
                Config::instance->m_video.grass_density = (*iter)->getCurrentValueIdx();
                Config::instance->m_video.terrain_detail = (*iter)->getCurrentValueIdx();
            }
        }

        if (id == "resolution")
        {
            const IntPair mode = Video::instance->getModes()[(*iter)->getCurrentValueIdx()];
            Config::instance->m_video.width = mode.first;
            Config::instance->m_video.height = mode.second;
        }
        else if (id == "fullscreen")
        {
            Config::instance->m_video.fullscreen = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "vsync")
        {
            Config::instance->m_video.vsync = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "fsaa_samples")
        {
            Config::instance->m_video.samples = 2 * (*iter)->getCurrentValueIdx();
        }
        else if (id == "use_shaders")
        {
            Config::instance->m_video.use_shaders = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "shadow_type")
        {
            Config::instance->m_video.shadow_type = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "shadowmap_size")
        {
            if (Video::instance->m_haveShadows)
            {
                Config::instance->m_video.shadowmap_size = (*iter)->getCurrentValueIdx();
            }
        }
        else if (id == "show_fps")
        {
            Config::instance->m_video.show_fps = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "grass_density")
        {
            Config::instance->m_video.grass_density = (*iter)->getCurrentValueIdx();
        }
        else if (id == "terrain_detail")
        {
            Config::instance->m_video.terrain_detail = (*iter)->getCurrentValueIdx();
        }
        else if (id == "audio")
        {
            Config::instance->m_audio.enabled = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "language")
        {
            StringVector languages = Language::instance->getAvailable();
            Config::instance->m_misc.language = languages[(*iter)->getCurrentValueIdx()];
        }
        else if (id == "mouse_sensitivity")
        {
            Config::instance->m_misc.mouse_sensitivity = 1.0f + static_cast<float>((*iter)->getCurrentValueIdx())/2.0f;
        }
        else if (id == "music_vol")
        {
            Config::instance->m_audio.music_vol = (*iter)->getCurrentValueIdx();
        }
        else if (id == "sound_vol")
        {
            Config::instance->m_audio.sound_vol = (*iter)->getCurrentValueIdx();
        }
    }
    
    m_menu->setState(State::Quit);
    g_needsToReload = true;
    g_optionsEntry = m_submenuToSwitchTo;
}

int OptionEntry::getMaxLeftWidth(const Font* font) const
{
    return font->getWidth(m_string) + font->getWidth(L":");
}

int OptionEntry::getMaxRightWidth(const Font* font) const
{
    return  font->getWidth(L"  ") +  m_value.getMaxWidth(font);
}
