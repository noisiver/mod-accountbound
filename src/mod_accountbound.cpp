#include "Player.h"
#include "ScriptMgr.h"

struct AccountBoundMounts
{
    uint32 SpellId;
    uint32 AllowableRace;
    uint32 AllowableClass;
    uint32 RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
};

struct FactionChangeMounts
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<AccountBoundMounts> accountBoundMounts;
std::vector<FactionChangeMounts> factionChangeMounts;

class AccountBoundPlayer : public PlayerScript
{
    public:
        AccountBoundPlayer() : PlayerScript("AccountBound") {}

        void OnLearnSpell(Player* player, uint32 spellID) override
        {
            for (auto& accountBoundMount : accountBoundMounts)
            {
                if (accountBoundMount.SpellId != spellID)
                    continue;

                int factionMountSpellId = GetFactionChangeMount(accountBoundMount.SpellId);
                if (factionMountSpellId == -1)
                {
                    LoginDatabase.DirectExecute("REPLACE INTO account_bound_mount (accountid, spellid, allowablerace, allowableclass, requiredlevel, "
                                                "requiredskill, requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {})",
                                                player->GetSession()->GetAccountId(),
                                                spellID,
                                                accountBoundMount.AllowableRace,
                                                accountBoundMount.AllowableClass,
                                                accountBoundMount.RequiredLevel,
                                                accountBoundMount.RequiredSkill,
                                                accountBoundMount.RequiredSkillRank);
                    continue;
                }

                LoginDatabase.DirectExecute("REPLACE INTO account_bound_mount (accountid, spellid, allowablerace, allowableclass, requiredlevel, requiredskill, "
                                            "requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {}), ({}, {}, {}, {}, {}, {}, {})",
                                            player->GetSession()->GetAccountId(),
                                            factionChangeMounts[factionMountSpellId].AllianceId,
                                            RACEMASK_ALLIANCE,
                                            accountBoundMount.AllowableClass,
                                            accountBoundMount.RequiredLevel,
                                            accountBoundMount.RequiredSkill,
                                            accountBoundMount.RequiredSkillRank,
                                            player->GetSession()->GetAccountId(),
                                            factionChangeMounts[factionMountSpellId].HordeId,
                                            RACEMASK_HORDE,
                                            accountBoundMount.AllowableClass,
                                            accountBoundMount.RequiredLevel,
                                            accountBoundMount.RequiredSkill,
                                            accountBoundMount.RequiredSkillRank);
            }
        }

        void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
        {
            LearnAccountBoundMounts(player);
        }

        void OnLogin(Player* player) override
        {
            LearnAccountBoundMounts(player);
        }

    private:
        void LearnAccountBoundMounts(Player* player)
        {
            QueryResult result = LoginDatabase.Query("SELECT spellid FROM account_bound_mount WHERE accountid={} AND allowablerace & {} "
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

        int GetFactionChangeMount(uint32 spellId)
        {
            for (size_t i = 0; i != factionChangeMounts.size(); ++i)
            {
                if (factionChangeMounts[i].AllianceId == spellId || factionChangeMounts[i].HordeId == spellId)
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
            LoadAccountBoundMounts();
            LoadAccountBoundFactionChangeMounts();
        }

    private:
        void LoadAccountBoundMounts()
        {
            QueryResult result = WorldDatabase.Query("SELECT spellid, allowablerace, allowableclass, requiredlevel, "
                                                     "requiredskill, requiredskillrank FROM account_bound_mount_template");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound spells");
                return;
            }

            accountBoundMounts.clear();

            int i = 0;
            do
            {
                Field* fields                           = result->Fetch();
                accountBoundMounts.push_back(AccountBoundMounts());
                accountBoundMounts[i].SpellId           = fields[0].Get<int32>();
                accountBoundMounts[i].AllowableRace     = fields[1].Get<int32>();
                accountBoundMounts[i].AllowableClass    = fields[2].Get<int32>();
                accountBoundMounts[i].RequiredLevel     = fields[3].Get<int32>();
                accountBoundMounts[i].RequiredSkill     = fields[4].Get<int32>();
                accountBoundMounts[i].RequiredSkillRank = fields[5].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound spells", i);
        }

        void LoadAccountBoundFactionChangeMounts()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
                                                     "account_bound_mount_template abt ON pfs.alliance_id = abt.spellid WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound faction spells");
                return;
            }

            factionChangeMounts.clear();

            int i = 0;
            do
            {
                Field* fields                          = result->Fetch();
                factionChangeMounts.push_back(FactionChangeMounts());
                factionChangeMounts[i].AllianceId      = fields[0].Get<int32>();
                factionChangeMounts[i].HordeId         = fields[1].Get<int32>();

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
