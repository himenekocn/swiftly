#include "core.h"

void SetupLuaConvars(LuaPlugin *plugin, lua_State *state)
{
    luabridge::getGlobalNamespace(state)
        .beginClass<PluginConvars>("ConVars")
        .addConstructor<void (*)(std::string)>()
        .addFunction("Get", &PluginConvars::GetConvarValueLua)
        .addFunction("GetType", &PluginConvars::GetConvarType)
        .addFunction("Set", &PluginConvars::SetConvar)
        .endClass();

    luaL_dostring(state, "convar = ConVars(GetCurrentPluginName())");
}