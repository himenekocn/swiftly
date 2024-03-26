#ifndef _player_grenadeparametersstashed_h
#define _player_grenadeparametersstashed_h

#include <stdint.h>
#include "../swiftly_memory.h"
#include "../swiftly_utils.h"

class GrenadeParametersStashed
{
private:
    uint32_t m_playerSlot;

public:
    GrenadeParametersStashed(uint32_t playerSlot) : m_playerSlot(playerSlot) {}
    ~GrenadeParametersStashed() {}

    bool Get()
    {
        REGISTER_METHOD(bool, false, scripting_Player_GetGrenadeParametersStashed, this->m_playerSlot);
    }

    void Set(bool val)
    {
        REGISTER_METHOD_VOID(scripting_Player_SetGrenadeParametersStashed, this->m_playerSlot, val);
    }
};

#endif