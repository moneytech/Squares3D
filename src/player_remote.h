#ifndef __PLAYER_REMOTE_H__
#define __PLAYER_REMOTE_H__

#include "common.h"
#include "player.h"

class ControlPacket;

class RemotePlayer : public Player
{
public:
    RemotePlayer(const Profile* profile, const Character* character, Level* level);
    ~RemotePlayer();

    void control();
    void control(const ControlPacket* packet);

private:
    int status; // ... from packet
};

#endif