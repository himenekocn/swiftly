#ifndef _signatures_h
#define _signatures_h

#include <map>
#include <string>

class CCSPlayerController;
class Z_CBaseEntity;

typedef void (*ClientPrint)(CCSPlayerController *, int, const char *, const char *, const char *, const char *, const char *);
typedef void (*NetworkStateChanged)(uintptr_t, int, int);
typedef void (*StateChanged)(void *, Z_CBaseEntity *, int, int, int);
typedef void (*CCSPlayerController_SwitchTeam)(CCSPlayerController *pController, unsigned int team);

class Signatures
{
private:
    std::map<std::string, void *> signatures;

public:
    void LoadSignatures();

    template <typename T>
    T FetchSignature(std::string name)
    {
        if (!this->Exists(name))
            return nullptr;

        return reinterpret_cast<T>(this->signatures.at(name));
    }

    void *FetchRawSignature(std::string name)
    {
        if (!this->Exists(name))
            return nullptr;

        return this->signatures.at(name);
    }

    bool Exists(std::string name);
};

extern Signatures *g_Signatures;

#endif