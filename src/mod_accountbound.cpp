#include "Player.h"
#include "ScriptMgr.h"

struct AccountBoundSpells
{
    uint32 SpellId;
    uint32 AllowableRace;
    uint32 AllowableClass;
    uint32 RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
};

struct FactionChangeSpells
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<AccountBoundSpells> accountBoundSpells;
std::vector<FactionChangeSpells> factionChangeSpells;

class AccountBoundPlayer : public PlayerScript
{
    public:
        AccountBoundPlayer() : PlayerScript("AccountBound") {}

        void OnLearnSpell(Player* player, uint32 spellID) override
        {
            for (auto& accountBoundSpell : accountBoundSpells)
            {
                if (accountBoundSpell.SpellId != spellID)
                    continue;

                int factionSpellId = GetFactionChangeSpell(accountBoundSpell.SpellId);
                if (factionSpellId == -1)
                {
                    LoginDatabase.DirectExecute("REPLACE INTO account_bound (accountid, spellid, allowablerace, allowableclass, requiredlevel, "
                                                "requiredskill, requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {})",
                                                player->GetSession()->GetAccountId(),
                                                spellID,
                                                accountBoundSpell.AllowableRace,
                                                accountBoundSpell.AllowableClass,
                                                accountBoundSpell.RequiredLevel,
                                                accountBoundSpell.RequiredSkill,
                                                accountBoundSpell.RequiredSkillRank);
                    continue;
                }

                LoginDatabase.DirectExecute("REPLACE INTO account_bound (accountid, spellid, allowablerace, allowableclass, requiredlevel, requiredskill, "
                                            "requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {}), ({}, {}, {}, {}, {}, {}, {})",
                                            player->GetSession()->GetAccountId(),
                                            factionChangeSpells[factionSpellId].AllianceId,
                                            RACEMASK_ALLIANCE,
                                            accountBoundSpell.AllowableClass,
                                            accountBoundSpell.RequiredLevel,
                                            accountBoundSpell.RequiredSkill,
                                            accountBoundSpell.RequiredSkillRank,
                                            player->GetSession()->GetAccountId(),
                                            factionChangeSpells[factionSpellId].HordeId,
                                            RACEMASK_HORDE,
                                            accountBoundSpell.AllowableClass,
                                            accountBoundSpell.RequiredLevel,
                                            accountBoundSpell.RequiredSkill,
                                            accountBoundSpell.RequiredSkillRank);
            }
        }

        void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
        {
            LearnAccountBound(player);
        }

        void OnLogin(Player* player) override
        {
            LearnAccountBound(player);
        }

    private:
        void LearnAccountBound(Player* player)
        {
            QueryResult result = LoginDatabase.Query("SELECT spellid FROM account_bound WHERE accountid={} AND allowablerace & {} "
                                                     "AND allowableclass & {} AND requiredlevel <= {} AND (requiredskill = 0 OR requiredskillrank <= {})",
                                                     player->GetSession()->GetAccountId(),
                                                     player->getRaceMask(),
                                                     player->getClassMask(),
                                                     player->getLevel(),
                                                     player->GetSkillValue(SKILL_RIDING));

            if (!result)
                return;

            do
            {
                Field* fields  = result->Fetch();
                uint32 spellId = fields[0].Get<uint32>();

                if (!player->HasSpell(spellId))
                    player->learnSpell(spellId);

            } while (result->NextRow());
        }

        int GetFactionChangeSpell(uint32 spellId)
        {
            for (size_t i = 0; i != factionChangeSpells.size(); ++i)
            {
                if (factionChangeSpells[i].AllianceId == spellId || factionChangeSpells[i].HordeId == spellId)
                    return i;
            }

            return -1;
        }
};

class AccountBoundWorld : public WorldScript
{
    public:
        AccountBoundWorld() : WorldScript("AccountBoundWorld") {}

        void OnStartup() override
        {
            LoadAccountBoundSpells();
            LoadAccountBoundFactionSpells();
        }

    private:
        void LoadAccountBoundSpells()
        {
            QueryResult result = WorldDatabase.Query("SELECT spellid, allowablerace, allowableclass, requiredlevel, "
                                                     "requiredskill, requiredskillrank FROM account_bound_template");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound spells");
                return;
            }

            accountBoundSpells.clear();

            int i = 0;
            do
            {
                Field* fields                           = result->Fetch();
                accountBoundSpells.push_back(AccountBoundSpells());
                accountBoundSpells[i].SpellId           = fields[0].Get<int32>();
                accountBoundSpells[i].AllowableRace     = fields[1].Get<int32>();
                accountBoundSpells[i].AllowableClass    = fields[2].Get<int32>();
                accountBoundSpells[i].RequiredLevel     = fields[3].Get<int32>();
                accountBoundSpells[i].RequiredSkill     = fields[4].Get<int32>();
                accountBoundSpells[i].RequiredSkillRank = fields[5].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound spells", i);
        }

        void LoadAccountBoundFactionSpells()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
                                                     "account_bound_template abt ON pfs.alliance_id = abt.spellid WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound faction spells");
                return;
            }

            factionChangeSpells.clear();

            int i = 0;
            do
            {
                Field* fields                          = result->Fetch();
                factionChangeSpells.push_back(FactionChangeSpells());
                factionChangeSpells[i].AllianceId      = fields[0].Get<int32>();
                factionChangeSpells[i].HordeId         = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound faction spells", i);
        }
};

void AddSC_mod_accountbound()
{
    new AccountBoundPlayer();
    new AccountBoundWorld();
}
