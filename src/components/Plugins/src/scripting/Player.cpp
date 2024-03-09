#include "../../../../common.h"
#include "../../../../player/PlayerManager.h"
#include "../../../../sdk/entity/CCSPlayerController.h"
#include "../../../../sdk/entity/CCSPlayerPawnBase.h"
#include "../../../../sdk/entity/CBaseCombatCharacter.h"
#include "../../inc/Scripting.h"
#include "../../../../utils/plat.h"
#include "../../../../commands/CommandsManager.h"

extern CEntitySystem *g_pEntitySystem;
std::string SerializeData(std::any data);

SMM_API const char *scripting_Player_GetName(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    return player->GetName();
}

SMM_API uint64 scripting_Player_GetSteamID(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    return player->GetSteamID();
}

SMM_API const char *scripting_Player_GetSteamID2(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "STEAM_0:0:000000000";

    uint64_t steamid = player->GetSteamID();
    if (steamid == 0)
        return "STEAM_0:0:000000000";

    static const uint64_t base = 76561197960265728;
    std::string data = string_format("STEAM_0:%d:%llu", (steamid - base) % 2, (steamid - base) / 2);

    char *result = new char[data.size() + 1];
    strcpy(result, data.c_str());
    return result;
}

SMM_API void scripting_Player_Drop(uint32 playerId, int reason)
{
    if (reason < 0 || reason > 69)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    if (!engine)
        return;

    engine->DisconnectClient(*player->GetSlot(), (ENetworkDisconnectionReason)reason);
}

SMM_API bool scripting_Player_IsAuthenticated(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return false;

    if (!engine)
        return false;

    return engine->IsClientFullyAuthenticated(*player->GetSlot());
}

SMM_API const char *scripting_Player_GetConvar(uint32 playerId, const char *name)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    if (!engine)
        return "";

    return engine->GetClientConVarValue(*player->GetSlot(), name);
}

SMM_API void scripting_Player_SendMessage(uint32 playerId, int dest, const char *text)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->SendMsg(dest, text);
}

SMM_API uint8 scripting_Player_GetTeam(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerController *controller = player->GetController();
    if (!controller)
        return CS_TEAM_NONE;

    return controller->m_iTeamNum();
}

SMM_API void scripting_Player_SetTeam(uint32 playerId, int team)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    if (team < 0 || team > 3)
        return;

    player->SwitchTeam(team);
}

SMM_API int scripting_Player_GetHealth(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return 0;

    return pawn->m_iHealth();
}

SMM_API void scripting_Player_SetHealth(uint32 playerId, int health)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    pawn->m_iHealth = health;
}

SMM_API int scripting_Player_GetMaxHealth(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return 0;

    return pawn->m_iMaxHealth();
}

SMM_API void scripting_Player_SetMaxHealth(uint32 playerId, int health)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    pawn->m_iMaxHealth = health;
}

SMM_API void scripting_Player_TakeHealth(uint32 playerId, int health)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    pawn->TakeDamage(health);
}

SMM_API void scripting_Player_Kill(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerPawn *pawn = player->GetPawn();
    if (!pawn)
        return;

    pawn->CommitSuicide(false, true);
}

SMM_API void scripting_Players_SendMessage(int dest, const char *text)
{
    for (uint16 i = 0; i < g_playerManager->GetPlayerCap(); i++)
    {
        Player *player = g_playerManager->GetPlayer(i);
        if (player == nullptr)
            continue;

        player->SendMsg(dest, text);
    }
}

SMM_API int scripting_Player_GetArmor(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CCSPlayerPawnBase *pPlayerPawn = player->GetPlayerBasePawn();
    if (!pPlayerPawn)
        return 0;

    return pPlayerPawn->m_ArmorValue();
}

SMM_API void scripting_Player_SetArmor(uint32 playerId, int armor)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawnBase *pPlayerPawn = player->GetPlayerBasePawn();
    if (!pPlayerPawn)
        return;

    pPlayerPawn->m_ArmorValue = (armor > 0 ? armor : 0);
}

SMM_API void scripting_Player_TakeArmor(uint32 playerId, int armor)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawnBase *pPlayerPawn = player->GetPlayerBasePawn();
    if (!pPlayerPawn)
        return;

    if (pPlayerPawn->m_ArmorValue() - armor < 0)
        pPlayerPawn->m_ArmorValue = 0;
    else
        pPlayerPawn->m_ArmorValue = pPlayerPawn->m_ArmorValue() - armor;
}

SMM_API const char *scripting_Player_GetClanTag(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    CCSPlayerController *pPlayerController = player->GetPlayerController();
    if (!pPlayerController)
        return "";

    return pPlayerController->m_szClan().String();
}

SMM_API void scripting_Player_SetClanTag(uint32 playerId, const char *tag)
{
    if (tag == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *pPlayerController = player->GetPlayerController();
    if (!pPlayerController)
        return;

    pPlayerController->m_szClan = CUtlSymbolLarge(tag);
}

SMM_API void scripting_Player_SetVar(uint32 playerId, const char *name, int type, ...)
{
    if (name == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    va_list ap;
    va_start(ap, type);

    if (type == 1)
        player->SetInternalVar(name, std::string(va_arg(ap, const char *)));
    else if (type == 2)
        player->SetInternalVar(name, va_arg(ap, int));
    else if (type == 3)
        player->SetInternalVar(name, va_arg(ap, unsigned int));
    else if (type == 4)
        player->SetInternalVar(name, va_arg(ap, double));
    else if (type == 5)
        player->SetInternalVar(name, va_arg(ap, long));
    else if (type == 6)
        player->SetInternalVar(name, va_arg(ap, unsigned long));
    else if (type == 7)
        player->SetInternalVar(name, (va_arg(ap, int) == 1));
    else if (type == 8)
        player->SetInternalVar(name, va_arg(ap, long long));
    else if (type == 9)
        player->SetInternalVar(name, va_arg(ap, unsigned long long));

    va_end(ap);
}

SMM_API const char *scripting_Player_GetVar(uint32 playerId, const char *name)
{
    if (name == nullptr)
        return "";

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    std::any value = player->GetInternalVar(name);

    std::string data = SerializeData(value);

    char *result = new char[data.size() + 1];
    strcpy(result, data.c_str());
    return result;
}

SMM_API uint32 scripting_Player_GetConnectedTime(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    return player->GetConnectedTime();
}

SMM_API int scripting_Player_FetchMatchStat(uint32 playerId, PlayerStat stat)
{
    if (stat < 0 || stat > 3)
        return 0;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CCSPlayerController *playerController = player->GetPlayerController();
    if (!playerController)
        return 0;

    CSMatchStats_t *matchStats = &playerController->m_pActionTrackingServices->m_matchStats();

    if (stat == PlayerStat::KILLS)
        return matchStats->m_iKills.Get();
    else if (stat == PlayerStat::DAMAGE)
        return matchStats->m_iDamage.Get();
    else if (stat == PlayerStat::ASSISTS)
        return matchStats->m_iAssists.Get();
    else if (stat == PlayerStat::DEATHS)
        return matchStats->m_iDeaths.Get();
    else
        return 0;
}

SMM_API void scripting_Player_SetMatchStat(uint32 playerId, PlayerStat stat, int value)
{
    if (stat < 0 || stat > 3)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *playerController = player->GetPlayerController();
    if (!playerController)
        return;

    if (stat == PlayerStat::KILLS)
        playerController->m_pActionTrackingServices->m_matchStats().m_iKills = value;
    else if (stat == PlayerStat::DAMAGE)
        playerController->m_pActionTrackingServices->m_matchStats().m_iDamage = value;
    else if (stat == PlayerStat::ASSISTS)
        playerController->m_pActionTrackingServices->m_matchStats().m_iAssists = value;
    else if (stat == PlayerStat::DEATHS)
        playerController->m_pActionTrackingServices->m_matchStats().m_iDeaths = value;
}

SMM_API const char *scripting_Player_GetIPAddress(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    return player->GetIPAddress().c_str();
}

SMM_API const char *scripting_Player_GetCoords(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    std::string data = SerializeData(player->GetCoords());

    char *result = new char[data.size() + 1];
    strcpy(result, data.c_str());
    return result;
}

SMM_API void scripting_Player_SetCoords(uint32 playerId, float x, float y, float z)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->SetCoords(x, y, z);
}

SMM_API const char *scripting_Player_GetVelocity(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    CBasePlayerPawn *pawn = player->GetPawn();
    if (!pawn)
        return "";

    std::string data = SerializeData(pawn->m_vecAbsVelocity());

    char *result = new char[data.size() + 1];
    strcpy(result, data.c_str());
    return result;
}

SMM_API void scripting_Player_SetVelocity(uint32 playerId, float x, float y, float z)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerPawn *pawn = player->GetPawn();
    if (!pawn)
        return;

    Vector vec(x, y, z);
    pawn->m_vecAbsVelocity = vec;
}

SMM_API int scripting_Player_GetMoney(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return 0;

    return controller->m_pInGameMoneyServices->m_iAccount();
}

SMM_API void scripting_Player_SetMoney(uint32 playerId, int money)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return;

    controller->m_pInGameMoneyServices->m_iAccount = money;
}

SMM_API void scripting_Player_TakeMoney(uint32 playerId, int money)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return;

    controller->m_pInGameMoneyServices->m_iAccount = controller->m_pInGameMoneyServices->m_iAccount() - money;
}

SMM_API void scripting_Player_Weapons_Drop(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CPlayer_WeaponServices *weaponServices = pawn->m_pWeaponServices();
    if (!weaponServices)
        return;

    CCSPlayer_ItemServices *itemServices = pawn->m_pItemServices();
    if (!itemServices)
        return;

    CUtlVector<CHandle<CBasePlayerWeapon>> *weapons = weaponServices->m_hMyWeapons();
    if (!weapons)
        return;

    FOR_EACH_VEC(*weapons, i)
    {
        CHandle<CBasePlayerWeapon> &weaponHandle = (*weapons)[i];
        if (!weaponHandle.IsValid())
            continue;

        CBasePlayerWeapon *weapon = weaponHandle.Get();
        if (!weapon)
            continue;

        weaponServices->m_hActiveWeapon = weaponHandle;
        itemServices->DropPlayerWeapon(weapon);
    }
}

SMM_API void scripting_Player_Weapons_Remove(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CPlayer_WeaponServices *weaponServices = pawn->m_pWeaponServices();
    if (!weaponServices)
        return;

    CCSPlayer_ItemServices *itemServices = pawn->m_pItemServices();
    if (!itemServices)
        return;

    CUtlVector<CHandle<CBasePlayerWeapon>> *weapons = weaponServices->m_hMyWeapons();
    if (!weapons)
        return;

    FOR_EACH_VEC(*weapons, i)
    {
        CHandle<CBasePlayerWeapon> &weaponHandle = (*weapons)[i];
        if (!weaponHandle.IsValid())
            continue;

        CBasePlayerWeapon *weapon = weaponHandle.Get();
        if (!weapon)
            continue;

        weaponServices->m_hActiveWeapon = weaponHandle;
        itemServices->DropPlayerWeapon(weapon);
        weaponServices->RemoveWeapon(weapon);
        weapon->Despawn();
    }
}

SMM_API void scripting_Player_Weapon_Remove(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CPlayer_WeaponServices *weaponServices = pawn->m_pWeaponServices();
    if (!weaponServices)
        return;

    CCSPlayer_ItemServices *itemServices = pawn->m_pItemServices();
    if (!itemServices)
        return;

    weaponServices->m_hActiveWeapon = weapon;
    itemServices->DropPlayerWeapon(weapon);
    weaponServices->RemoveWeapon(weapon);
    weapon->Despawn();
}

SMM_API void scripting_Player_Weapon_SetClipAmmo(uint32 playerId, uint32 slot, int ammo)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    CCSWeaponBaseVData *vData = weapon->GetWeaponVData();
    if (vData)
    {
        vData->m_iMaxClip1 = ammo;
        vData->m_iDefaultClip1 = ammo;
    }

    weapon->m_iClip1 = ammo;
}

SMM_API void scripting_Player_Weapon_SetClip2Ammo(uint32 playerId, uint32 slot, int ammo)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    CCSWeaponBaseVData *vData = weapon->GetWeaponVData();
    if (vData)
    {
        vData->m_iMaxClip2 = ammo;
        vData->m_iDefaultClip2 = ammo;
    }

    weapon->m_iClip2 = ammo;
}

SMM_API int32_t scripting_Player_Weapon_GetNextPrimaryAttackTick(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0;

    return weapon->m_nNextPrimaryAttackTick();
}

SMM_API int32_t scripting_Player_Weapon_GetNextSecondaryAttackTick(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0;

    return weapon->m_nNextSecondaryAttackTick();
}

SMM_API void scripting_Player_Weapon_SetNextPrimaryAttackTick(uint32 playerId, uint32 slot, int32_t tick)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_nNextPrimaryAttackTick = tick;
}

SMM_API void scripting_Player_Weapon_SetNextSecondaryAttackTick(uint32 playerId, uint32 slot, int32_t tick)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_nNextSecondaryAttackTick = tick;
}

SMM_API float scripting_Player_Weapon_GetNextPrimaryAttackTickRatio(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0.0f;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0.0f;

    return weapon->m_flNextPrimaryAttackTickRatio();
}

SMM_API float scripting_Player_Weapon_GetNextSecondaryAttackTickRatio(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0.0f;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0.0f;

    return weapon->m_flNextSecondaryAttackTickRatio();
}

SMM_API void scripting_Player_Weapon_SetNextPrimaryAttackTickRatio(uint32 playerId, uint32 slot, float ratio)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_flNextPrimaryAttackTickRatio = ratio;
}

SMM_API void scripting_Player_Weapon_SetNextSecondaryAttackTickRatio(uint32 playerId, uint32 slot, float ratio)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_flNextSecondaryAttackTickRatio = ratio;
}

SMM_API uint32_t scripting_Player_Weapon_GetSilencerType(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return CSWeaponSilencerType::WEAPONSILENCER_NONE;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return CSWeaponSilencerType::WEAPONSILENCER_NONE;

    CCSWeaponBaseVData *vData = weapon->GetWeaponVData();
    if (!vData)
        return CSWeaponSilencerType::WEAPONSILENCER_NONE;

    return vData->m_eSilencerType();
}

SMM_API void scripting_Player_Weapons_SetActiveWeapon(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromSlot((gear_slot_t)slot);
    if (!weapon)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CPlayer_WeaponServices *weaponServices = pawn->m_pWeaponServices();
    if (!weaponServices)
        return;

    CCSPlayer_ItemServices *itemServices = pawn->m_pItemServices();
    if (!itemServices)
        return;

    weaponServices->m_hActiveWeapon = weapon;
}

SMM_API void scripting_Player_Weapon_SetReserveAmmo(uint32 playerId, uint32 slot, int ammo)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    CCSWeaponBaseVData *vData = weapon->GetWeaponVData();
    if (vData)
    {
        vData->m_nPrimaryReserveAmmoMax = ammo;
    }

    weapon->m_pReserveAmmo[0] = ammo;
}

SMM_API void scripting_Player_Weapon_Drop(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CPlayer_WeaponServices *weaponServices = pawn->m_pWeaponServices();
    if (!weaponServices)
        return;

    CCSPlayer_ItemServices *itemServices = pawn->m_pItemServices();
    if (!itemServices)
        return;

    weaponServices->m_hActiveWeapon = weapon;
    itemServices->DropPlayerWeapon(weapon);
}

SMM_API void scripting_Player_Weapon_SetStatTrack(uint32 playerId, uint32 slot, bool stattrack)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_nFallbackStatTrak = (int)stattrack;
}

SMM_API void scripting_Player_Weapon_SetWear(uint32 playerId, uint32 slot, float wear)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_flFallbackWear = wear;
}
SMM_API void scripting_Player_Weapon_SetPaintKit(uint32 playerId, uint32 slot, int paintkit)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_nFallbackPaintKit = paintkit;
}

SMM_API void scripting_Player_Weapon_SetSeed(uint32 playerId, uint32 slot, int seed)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_nFallbackSeed = seed;
}

SMM_API bool scripting_Player_Weapon_GetStatTrack(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return false;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return false;

    return (weapon->m_nFallbackStatTrak() != 0);
}
SMM_API float scripting_Player_Weapon_GetWear(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0.0f;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0.0f;

    return weapon->m_flFallbackWear();
}
SMM_API int scripting_Player_Weapon_GetPaintKit(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0;

    return weapon->m_nFallbackPaintKit();
}
SMM_API int scripting_Player_Weapon_GetSeed(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return 0;

    return weapon->m_nFallbackSeed();
}

SMM_API uint32 scripting_Player_Weapon_GetType(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return gear_slot_t::GEAR_SLOT_INVALID;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return gear_slot_t::GEAR_SLOT_INVALID;

    return weapon->GetWeaponVData()->m_GearSlot();
}

SMM_API const char *scripting_Player_Weapon_GetName(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return "";

    return weapon->GetClassname();
}

SMM_API void scripting_Player_SetFlashDuration(uint32 playerId, float duration)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawnBase *pawn = player->GetPlayerBasePawn();
    if (!pawn)
        return;

    pawn->m_flFlashDuration = duration;
}

SMM_API void scripting_Player_Weapons_Give(uint32 playerId, const char *name)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CCSPlayer_ItemServices *itemServices = pawn->m_pItemServices();
    if (!itemServices)
        return;

    std::string n = std::string(name);

    if (n == "item_defuser" && player->GetController() && player->GetController()->m_iTeamNum() == CS_TEAM_CT)
        itemServices->m_bHasDefuser = true;
    else if (n == "item_assaultsuit")
    {
        itemServices->m_bHasHelmet = true;
        scripting_Player_SetArmor(playerId, 100);
    }
    else if (n == "item_kevlar")
    {
        scripting_Player_SetArmor(playerId, 100);
    }
    else
        pawn->GiveNamedItem(name);
}

SMM_API uint32_t scripting_Player_Weapons_GetWeaponID(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromSlot((gear_slot_t)slot);
    if (!weapon)
        return 0;

    return weapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex();
}

SMM_API bool scripting_Player_Weapon_Exists(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return false;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    return (weapon != nullptr);
}

std::vector<uint16_t> paintkitsFallbackCheck = {1171, 1170, 1169, 1164, 1162, 1161, 1159, 1175, 1174, 1167, 1165, 1168, 1163, 1160, 1166, 1173};

SMM_API void scripting_Player_Weapon_SetDefaultChangeSkinAttributes(uint32 playerId, uint32 slot)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    weapon->m_AttributeManager().m_Item().m_iItemIDHigh = -1;

    if (weapon->GetWeaponVData()->m_GearSlot == gear_slot_t::GEAR_SLOT_KNIFE)
    {
        weapon->m_AttributeManager().m_Item().m_iEntityQuality = 3;
    }

    int paintkit = weapon->m_nFallbackPaintKit();
    bool legacy = (std::find(paintkitsFallbackCheck.begin(), paintkitsFallbackCheck.end(), paintkit) == paintkitsFallbackCheck.end());

    if (weapon->m_CBodyComponent() && weapon->m_CBodyComponent()->m_pSceneNode())
    {
        CSkeletonInstance *skeleton = weapon->m_CBodyComponent()->m_pSceneNode()->GetSkeletonInstance();
        if (skeleton && skeleton->m_modelState())
            if (skeleton->m_modelState()->m_MeshGroupMask() != (legacy == true ? 2 : 1))
            {
                skeleton->m_modelState()->m_MeshGroupMask = (legacy == true ? 2 : 1);
            }
    }

    CCSPlayer_ViewModelServices *viewmodelServices = pawn->m_pViewModelServices();
    if (!viewmodelServices)
        return;

    CHandle<CBaseViewModel> *viewmodelHandles = viewmodelServices->m_hViewModel();
    if (!viewmodelHandles)
        return;

    CBaseViewModel *viewmodel = viewmodelHandles[0];

    if (!viewmodel)
        return;

    if (viewmodel->m_CBodyComponent() && viewmodel->m_CBodyComponent()->m_pSceneNode())
    {
        CSkeletonInstance *viewmodelskeleton = viewmodel->m_CBodyComponent()->m_pSceneNode()->GetSkeletonInstance();
        if (viewmodelskeleton && viewmodelskeleton->m_modelState())
            if (viewmodelskeleton->m_modelState()->m_MeshGroupMask() != (legacy == true ? 2 : 1))
            {
                viewmodelskeleton->m_modelState()->m_MeshGroupMask = (legacy == true ? 2 : 1);
            }
    }

    viewmodel->m_CBodyComponent.StateUpdate();
}

SMM_API void scripting_Player_Weapon_SetNametag(uint32 playerId, uint32 slot, const char *text)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerWeapon *weapon = player->GetPlayerWeaponFromID(slot);
    if (!weapon)
        return;

    auto customNamePtr = weapon->m_AttributeManager().m_Item().m_szCustomName();
    Plat_WriteMemory((void *)customNamePtr, reinterpret_cast<byte *>(const_cast<char *>(text)), strlen(text));
}

SMM_API void scripting_Player_ExecuteCommand(uint32 playerId, const char *cmd)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    if (starts_with(cmd, "sw_"))
    {
        CCommand tokenizedArgs;
        tokenizedArgs.Tokenize(cmd);

        std::vector<std::string> cmdString;
        for (int i = 1; i < tokenizedArgs.ArgC(); i++)
            cmdString.push_back(tokenizedArgs[i]);

        std::string commandName = replace(tokenizedArgs[0], "sw_", "");

        Command *command = g_commandsManager->FetchCommand(commandName);
        if (command)
            command->Exec(player->GetSlot()->Get(), cmdString, true);
    }
    else
        engine->ClientCommand(*player->GetSlot(), cmd);
}

SMM_API void scripting_Player_SetModel(uint32 playerId, const char *model)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    bool hasModel = (g_precacher->HasModelInList(model));
    if (!hasModel)
    {
        g_precacher->AddModel(model);
        PRINTF("Precacher", "Model '%s' was not precached before and it was added to the list.\n", model);
        PRINT("Precacher", "It may work on the second map change if the model is valid.\n");
    }

    pawn->SetModel(model);
}

SMM_API void scripting_Player_SetMusicKit(uint32 playerId, int musicid)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return;

    CCSPlayerController_InventoryServices *inventory = controller->m_pInventoryServices();
    if (!inventory)
        return;

    inventory->m_unMusicID = musicid;
}

SMM_API void scripting_Player_ShowMenu(uint32 playerId, const char *menuid)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    if (player->HasMenuShown())
        return;

    player->ShowMenu(menuid);
}

SMM_API void scripting_Player_HideMenu(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    if (!player->HasMenuShown())
        return;

    player->HideMenu();
}

SMM_API int scripting_Player_GetLatency(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return -1;
    if (player->IsFakeClient())
        return 0;

    INetChannelInfo *netinfo = engine->GetPlayerNetInfo(*player->GetSlot());

    return netinfo->GetLatency(FLOW_INCOMING) + netinfo->GetLatency(FLOW_OUTGOING);
}

SMM_API void scripting_Player_SetGloves(uint32 playerId, uint16_t defindex, int paintkit, int seed, float wear)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    CEconItemView *gloves_view = pawn->m_EconGloves();

    gloves_view->m_iItemDefinitionIndex = defindex;
    gloves_view->m_iItemIDLow = -1;
    gloves_view->m_iItemIDHigh = (16384 & 0xFFFFFFFF);

    gloves_view->m_bInitialized = true;

    g_Plugin.NextFrame([gloves_view, paintkit, seed, wear, pawn]() -> void
                       {
            g_Signatures->FetchSignature<CAttributeList_SetOrAddAttributeValueByName>("CAttributeList_SetOrAddAttributeValueByName")(gloves_view->m_NetworkedDynamicAttributes(), "set item texture prefab", paintkit);
            g_Signatures->FetchSignature<CAttributeList_SetOrAddAttributeValueByName>("CAttributeList_SetOrAddAttributeValueByName")(gloves_view->m_NetworkedDynamicAttributes(), "set item texture seed", static_cast<float>(seed));
            g_Signatures->FetchSignature<CAttributeList_SetOrAddAttributeValueByName>("CAttributeList_SetOrAddAttributeValueByName")(gloves_view->m_NetworkedDynamicAttributes(), "set item texture wear", wear);

            g_Signatures->FetchSignature<CBaseModelEntity_SetBodygroup>("CBaseModelEntity_SetBodygroup")(pawn, "default_gloves", 1LL); });
}

SMM_API float scripting_Player_GetGravity(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0.0f;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return 0.0f;

    return pawn->m_flGravityScale();
}

SMM_API void scripting_Player_SetGravity(uint32 playerId, float gravity)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    pawn->m_flGravityScale = gravity;
}

SMM_API float scripting_Player_GetSpeed(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0.0f;

    CCSPlayerPawnBase *pawn = player->GetPlayerBasePawn();
    if (!pawn)
        return 0.0f;

    return pawn->m_flVelocityModifier();
}

SMM_API void scripting_Player_SetSpeed(uint32 playerId, float speed)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawnBase *pawn = player->GetPlayerBasePawn();
    if (!pawn)
        return;

    pawn->m_flVelocityModifier = speed;
}

QAngle scripting_Player_GetEyeAnglesRaw(uint32 playerId)
{
    QAngle angle(0.0f, 0.0f, 0.0f);

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return angle;

    CCSPlayerPawnBase *pawn = player->GetPlayerBasePawn();
    if (!pawn)
        return angle;

    return pawn->m_angEyeAngles();
}

SMM_API const char *scripting_Player_GetEyeAngles(uint32 playerId)
{
    std::string data = SerializeData(scripting_Player_GetEyeAnglesRaw(playerId));

    char *result = new char[data.size() + 1];
    strcpy(result, data.c_str());
    return result;
}

SMM_API void scripting_Player_SetEyeAngles(uint32 playerId, float x, float y, float z)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawnBase *pawn = player->GetPlayerBasePawn();
    if (!pawn)
        return;

    QAngle angle(x, y, z);
    pawn->m_angEyeAngles = angle;
}

SMM_API void scripting_Player_Respawn(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return;

    controller->Respawn();
}

SMM_API void scripting_Player_SetDesiredFOV(uint32 playerId, uint32_t fov)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerController *controller = player->GetController();
    if (!controller)
        return;

    controller->m_iDesiredFOV = fov;
}

SMM_API uint32_t scripting_Player_GetDesiredFOV(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return 0;

    CBasePlayerController *controller = player->GetController();
    if (!controller)
        return 0;

    return controller->m_iDesiredFOV();
}

SMM_API void scripting_Player_SetHealthShotBoostEffectExpirationTime(uint32 playerId, float expireTime)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerPawn *pawn = player->GetPlayerPawn();
    if (!pawn)
        return;

    pawn->m_flHealthShotBoostExpirationTime = expireTime;
}

SMM_API void scripting_Player_SetConvar(uint32 playerId, const char *cvarname, const char *cvarvalue)
{
    if (cvarname == nullptr || cvarvalue == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->SetClientConvar(cvarname, cvarvalue);
}

SMM_API const char *scripting_Player_GetChatTag(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return "";

    std::string data = player->tag;

    char *result = new char[data.size() + 1];
    strcpy(result, data.c_str());
    return result;
}

SMM_API void scripting_Player_SetChatTag(uint32 playerId, const char *tag)
{
    if (tag == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->tag = tag;
}

SMM_API void scripting_Player_SetChatTagColor(uint32 playerId, const char *tagcolor)
{
    if (tagcolor == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->tagcolor = tagcolor;
}

SMM_API void scripting_Player_SetNameColor(uint32 playerId, const char *namecolor)
{
    if (namecolor == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->namecolor = namecolor;
}

SMM_API void scripting_Player_SetChatColor(uint32 playerId, const char *chatcolor)
{
    if (chatcolor == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    player->chatcolor = chatcolor;
}

SMM_API void scripting_Player_SetName(uint32 playerId, const char *name)
{
    if (name == nullptr)
        return;

    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CBasePlayerController *controller = player->GetController();
    if (!controller)
        return;

    auto namePtr = controller->m_iszPlayerName();
    Plat_WriteMemory((void *)namePtr, reinterpret_cast<byte *>(const_cast<char *>(name)), strlen(name));
}

SMM_API bool scripting_Player_IsFirstSpawn(uint32 playerId)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return false;

    return player->IsFirstSpawn();
}

SMM_API void scripting_Player_SetCompetitiveRanking(uint32 playerId, int ranking)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return;

    controller->m_iCompetitiveRanking = ranking;
}

SMM_API void scripting_Player_SetCompetitiveRankType(uint32 playerId, int type)
{
    Player *player = g_playerManager->GetPlayer(playerId);
    if (!player)
        return;

    CCSPlayerController *controller = player->GetPlayerController();
    if (!controller)
        return;

    controller->m_iCompetitiveRankType = type;
}