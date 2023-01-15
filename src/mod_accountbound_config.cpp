#include "Config.h"

#include "mod_accountbound.h"

void AccountBound::OnAfterConfigLoad(bool /*reload*/)
{
    EnableGamemasters = sConfigMgr->GetOption<bool>("AccountBound.Gamemasters", 0);
    EnableAccountCompanions = sConfigMgr->GetOption<bool>("AccountBound.Companions", 1);
    EnableAccountHeirlooms = sConfigMgr->GetOption<bool>("AccountBound.Heirlooms", 0);
    EnableAccountMounts = sConfigMgr->GetOption<bool>("AccountBound.Mounts", 1);
}
