#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "vmath.h"
#include "state.h"
#include "system.h"

const float DT = 0.01f;

class Config;
class Language;
class Video;
class Audio;
class Network;
class Input;
class FPS;
class Profile;

extern bool   g_needsToReload;
extern string g_optionsEntry;

typedef vector<Profile*> ProfilesVector;

class Game : public System<Game>, public NoCopy
{
public:
    Game();
    ~Game();
    
    void run();
    void saveScreenshot(const FPS& fps) const;

    // Singletons
    Config*     m_config;
    Language*   m_language;
    Video*      m_video;
    Audio*      m_audio;
    Network*    m_network;
    Input*      m_input;
    //

    ProfilesVector  m_cpuProfiles[4];
    Profile*        m_userProfile;    

private:
    State*      m_state;

    bool        m_fixedTimestep;
    int         m_unlockable;
    int         m_current;
    bool        m_screenLast;

    State* switchState(const State::Type newState);
    void loadCpuData();
    void saveUserData();
    void loadUserData();
};

#endif
