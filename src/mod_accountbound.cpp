#include "Player.h"
#include "ScriptMgr.h"

class AccountBound : public PlayerScript
{
    public:
        AccountBound() : PlayerScript("AccountBound") {}
};

void AddSC_mod_accountbound()
{
    new AccountBound();
}
