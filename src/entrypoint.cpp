#include <stdio.h>
#include "common.h"
#include "iserver.h"

#include "hooks/Hooks.h"
#include "components/test_component/inc/TestComponent.h"

#define LOAD_COMPONENT(TYPE, VARIABLE_NAME) \
    {                                       \
        TYPE *VARIABLE_NAME = new TYPE();   \
        VARIABLE_NAME->LoadComponent();     \
    }

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK4_void(IServerGameClients, ClientActive, SH_NOATTRIB, 0, CPlayerSlot, bool, const char *, uint64);
SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, int, const char *, uint64, const char *);
SH_DECL_HOOK4_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);
SH_DECL_HOOK1_void(IServerGameClients, ClientSettingsChanged, SH_NOATTRIB, 0, CPlayerSlot);
SH_DECL_HOOK6_void(IServerGameClients, OnClientConnected, SH_NOATTRIB, 0, CPlayerSlot, const char *, uint64, const char *, const char *, bool);
SH_DECL_HOOK6(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, CPlayerSlot, const char *, uint64, const char *, bool, CBufferString *);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, CPlayerSlot, const CCommand &);

EventMap eventMap;
SwiftlyPlugin g_Plugin;
IServerGameDLL *server = NULL;
IServerGameClients *gameclients = NULL;
IVEngineServer2 *engine = NULL;
IFileSystem *filesystem = NULL;
ICvar *icvar = NULL;

CGlobalVars *GetGameGlobals()
{
    INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

    if (!server)
        return nullptr;

    return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

PLUGIN_EXPOSE(SwiftlyPlugin, g_Plugin);
bool SwiftlyPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
    GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

    META_CONPRINTF("Loading hooks...\n");

    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &SwiftlyPlugin::Hook_GameFrame, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &SwiftlyPlugin::Hook_ClientActive, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &SwiftlyPlugin::Hook_ClientDisconnect, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &SwiftlyPlugin::Hook_ClientPutInServer, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &SwiftlyPlugin::Hook_ClientSettingsChanged, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, gameclients, this, &SwiftlyPlugin::Hook_OnClientConnected, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &SwiftlyPlugin::Hook_ClientConnect, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &SwiftlyPlugin::Hook_ClientCommand, false);

    META_CONPRINTF("All hooks has been loaded!\n");

    META_CONPRINTF("Loading components...\n");

    LOAD_COMPONENT(TestComponent, test_component);

    META_CONPRINTF("All components has been loaded!\n");

    g_pCVar = icvar;
    ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_GAMEDLL);

    return true;
}

void SwiftlyPlugin::AllPluginsLoaded()
{
}

bool SwiftlyPlugin::Unload(char *error, size_t maxlen)
{
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &SwiftlyPlugin::Hook_GameFrame, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &SwiftlyPlugin::Hook_ClientActive, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &SwiftlyPlugin::Hook_ClientDisconnect, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &SwiftlyPlugin::Hook_ClientPutInServer, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &SwiftlyPlugin::Hook_ClientSettingsChanged, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, gameclients, this, &SwiftlyPlugin::Hook_OnClientConnected, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &SwiftlyPlugin::Hook_ClientConnect, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &SwiftlyPlugin::Hook_ClientCommand, false);

    ConVar_Unregister();
    return true;
}

void SwiftlyPlugin::Hook_ClientActive(CPlayerSlot slot, bool bLoadGame, const char *pszName, uint64 xuid)
{
    META_CONPRINTF("Hook_ClientActive(%d, %d, \"%s\", %d)\n", slot, bLoadGame, pszName, xuid);
}

void SwiftlyPlugin::Hook_ClientCommand(CPlayerSlot slot, const CCommand &args)
{
    META_CONPRINTF("Hook_ClientCommand(%d, \"%s\")\n", slot, args.GetCommandString());
    hooks::emit(OnClientCommand(&slot, &args));
}

void SwiftlyPlugin::Hook_ClientSettingsChanged(CPlayerSlot slot)
{
    META_CONPRINTF("Hook_ClientSettingsChanged(%d)\n", slot);
}

void SwiftlyPlugin::Hook_OnClientConnected(CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, const char *pszAddress, bool bFakePlayer)
{
    hooks::emit(OnClientConnectedEvent(&slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer));
}

bool SwiftlyPlugin::Hook_ClientConnect(CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, bool unk1, CBufferString *pRejectReason)
{
    META_CONPRINTF("Hook_ClientConnect(%d, \"%s\", %d, \"%s\", %d, \"%s\")\n", slot, pszName, xuid, pszNetworkID, unk1, pRejectReason->ToGrowable()->Get());

    RETURN_META_VALUE(MRES_IGNORED, true);
}

void SwiftlyPlugin::Hook_ClientPutInServer(CPlayerSlot slot, char const *pszName, int type, uint64 xuid)
{
    META_CONPRINTF("Hook_ClientPutInServer(%d, \"%s\", %d, %d, %d)\n", slot, pszName, type, xuid);
}

void SwiftlyPlugin::Hook_ClientDisconnect(CPlayerSlot slot, int reason, const char *pszName, uint64 xuid, const char *pszNetworkID)
{
    META_CONPRINTF("Hook_ClientDisconnect(%d, %d, \"%s\", %d, \"%s\")\n", slot, reason, pszName, xuid, pszNetworkID);
}

void SwiftlyPlugin::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
}

// Potentially might not work
void SwiftlyPlugin::OnLevelInit(char const *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background)
{
    META_CONPRINTF("OnLevelInit(%s)\n", pMapName);
}

// Potentially might not work
void SwiftlyPlugin::OnLevelShutdown()
{
    META_CONPRINTF("OnLevelShutdown()\n");
}

bool SwiftlyPlugin::Pause(char *error, size_t maxlen)
{
    return true;
}

bool SwiftlyPlugin::Unpause(char *error, size_t maxlen)
{
    return true;
}

const char *SwiftlyPlugin::GetLicense()
{
    return "MIT License";
}

const char *SwiftlyPlugin::GetVersion()
{
    return "1.0.0";
}

const char *SwiftlyPlugin::GetDate()
{
    return __DATE__;
}

const char *SwiftlyPlugin::GetLogTag()
{
    return "SWIFTLY";
}

const char *SwiftlyPlugin::GetAuthor()
{
    return "Swiftly Team";
}

const char *SwiftlyPlugin::GetDescription()
{
    return "Swiftly - Plugin";
}

const char *SwiftlyPlugin::GetName()
{
    return "Swiftly";
}

const char *SwiftlyPlugin::GetURL()
{
    return "https://";
}
