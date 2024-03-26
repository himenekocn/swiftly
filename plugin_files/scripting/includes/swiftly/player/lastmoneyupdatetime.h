#ifndef _player_lastmoneyupdatetime_h
#define _player_lastmoneyupdatetime_h

#include <stdint.h>
#include "../swiftly_memory.h"
#include "../swiftly_utils.h"

class LastMoneyUpdateTime
{
private:
    uint32_t m_playerSlot;

public:
    LastMoneyUpdateTime(uint32_t playerSlot) : m_playerSlot(playerSlot) {}
    ~LastMoneyUpdateTime() {}

    float Get()
    {
        REGISTER_METHOD(float, 0.0f, scripting_Player_GetLastMoneyUpdateTime, this->m_playerSlot);
    }

    void Set(float val)
    {
        REGISTER_METHOD_VOID(scripting_Player_SetLastMoneyUpdateTime, this->m_playerSlot, val);
    }
};

#endif