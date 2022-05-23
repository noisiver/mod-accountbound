#include "Player.h"
#include "ScriptMgr.h"

struct AccountMounts
{
    uint32 SpellId;
    uint32 AllowableRace;
    uint32 AllowableClass;
    uint32 RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
};

struct FactionSpecificAccountMounts
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<AccountMounts> accountMounts;
std::vector<FactionSpecificAccountMounts> factionSpecificAccountMounts;

class AccountBoundMounts : public PlayerScript
{
    public:
        AccountBoundMounts() : PlayerScript("AccountBoundMounts") {}

        void OnLearnSpell(Player* player, uint32 spellID) override
        {
            SaveMounts(player, spellID);
        }

        void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
        {
            LearnMounts(player);
        }

        void OnLogin(Player* player) override
        {
            LearnMounts(player);
        }
    private:
        void SaveMounts(Player* player, uint32 spellID)
        {
            for (auto& mount : accountMounts)
            {
                if (mount.SpellId != spellID)
                    continue;

                int factionSpecificMountSpellId = FindFactionSpecificMount(mount.SpellId);
                if (factionSpecificMountSpellId == -1)
                {
                    LoginDatabase.DirectExecute("REPLACE INTO account_bound_mounts (accountid, spellid, allowablerace, allowableclass, requiredlevel, "
                                                "requiredskill, requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {})",
                                                player->GetSession()->GetAccountId(),
                                                spellID,
                                                mount.AllowableRace,
                                                mount.AllowableClass,
                                                mount.RequiredLevel,
                                                mount.RequiredSkill,
                                                mount.RequiredSkillRank);
                    continue;
                }

                LoginDatabase.DirectExecute("REPLACE INTO account_bound_mounts (accountid, spellid, allowablerace, allowableclass, requiredlevel, requiredskill, "
                                            "requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {}), ({}, {}, {}, {}, {}, {}, {})",
                                            player->GetSession()->GetAccountId(),
                                            factionSpecificAccountMounts[factionSpecificMountSpellId].AllianceId,
                                            RACEMASK_ALLIANCE,
                                            mount.AllowableClass,
                                            mount.RequiredLevel,
                                            mount.RequiredSkill,
                                            mount.RequiredSkillRank,
                                            player->GetSession()->GetAccountId(),
                                            factionSpecificAccountMounts[factionSpecificMountSpellId].HordeId,
                                            RACEMASK_HORDE,
                                            mount.AllowableClass,
                                            mount.RequiredLevel,
                                            mount.RequiredSkill,
                                            mount.RequiredSkillRank);
            }
        }

        void LearnMounts(Player* player)
        {
            QueryResult result = LoginDatabase.Query("SELECT spellid FROM account_bound_mounts WHERE accountid={} AND allowablerace & {} "
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

        int FindFactionSpecificMount(uint32 spellId)
        {
            for (size_t i = 0; i != factionSpecificAccountMounts.size(); ++i)
            {
                if (factionSpecificAccountMounts[i].AllianceId == spellId || factionSpecificAccountMounts[i].HordeId == spellId)
                    return i;
            }

            return -1;
        }
};

struct AccountCompanions
{
    uint32 SpellId;
    uint32 AllowableRace;
};

struct FactionSpecificAccountCompanions
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<AccountCompanions> accountCompanions;
std::vector<FactionSpecificAccountCompanions> factionSpecificAccountCompanions;

class AccountBoundCompanions : public PlayerScript
{
    public:
        AccountBoundCompanions() : PlayerScript("AccountBoundCompanions") {}

        void OnLearnSpell(Player* player, uint32 spellID) override
        {
            SaveCompanions(player, spellID);
        }

        void OnLogin(Player* player) override
        {
            LoadCompanions(player);
        }
    private:
        void SaveCompanions(Player* player, uint32 spellID)
        {
            for (auto& accountBoundCompanion : accountCompanions)
            {
                if (accountBoundCompanion.SpellId != spellID)
                    continue;

                int factionCompanionSpellId = FindFactionSpecificCompanion(accountBoundCompanion.SpellId);
                if (factionCompanionSpellId == -1)
                {
                    LoginDatabase.DirectExecute("REPLACE INTO account_bound_companions (accountid, spellid, allowablerace)"
                                                "VALUES ({}, {}, {})",
                                                player->GetSession()->GetAccountId(),
                                                spellID,
                                                accountBoundCompanion.AllowableRace);
                    continue;
                }

                LoginDatabase.DirectExecute("REPLACE INTO account_bound_companions (accountid, spellid, allowablerace) "
                                            "VALUES ({}, {}, {}), ({}, {}, {})",
                                            player->GetSession()->GetAccountId(),
                                            factionSpecificAccountCompanions[factionCompanionSpellId].AllianceId,
                                            RACEMASK_ALLIANCE,
                                            player->GetSession()->GetAccountId(),
                                            factionSpecificAccountCompanions[factionCompanionSpellId].HordeId,
                                            RACEMASK_HORDE);
            }
        }

        void LoadCompanions(Player* player)
        {
            QueryResult result = LoginDatabase.Query("SELECT spellid FROM account_bound_companions WHERE accountid={} AND allowablerace & {}",
                                                     player->GetSession()->GetAccountId(),
                                                     player->getRaceMask());

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

        int FindFactionSpecificCompanion(uint32 spellId)
        {
            for (size_t i = 0; i != factionSpecificAccountCompanions.size(); ++i)
            {
                if (factionSpecificAccountCompanions[i].AllianceId == spellId || factionSpecificAccountCompanions[i].HordeId == spellId)
                    return i;
            }

            return -1;
        }
};

class AccountBoundData : public WorldScript
{
    public:
        AccountBoundData() : WorldScript("AccountBoundData") {}

        void OnStartup() override
        {
            LOG_INFO("server.loading", ">> Loading account bound templates...");
            LoadMounts();
            LoadFactionSpecificMounts();
            LoadCompanions();
            LoadFactionSpecificCompanions();
        }

    private:
        void LoadMounts()
        {
            QueryResult result = WorldDatabase.Query("SELECT spellid, allowablerace, allowableclass, requiredlevel, "
                                                     "requiredskill, requiredskillrank FROM account_bound_mount_template");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 mount spells");
                return;
            }

            accountMounts.clear();

            int i = 0;
            do
            {
                Field* fields               = result->Fetch();
                accountMounts.push_back(AccountMounts());
                accountMounts[i].SpellId           = fields[0].Get<int32>();
                accountMounts[i].AllowableRace     = fields[1].Get<int32>();
                accountMounts[i].AllowableClass    = fields[2].Get<int32>();
                accountMounts[i].RequiredLevel     = fields[3].Get<int32>();
                accountMounts[i].RequiredSkill     = fields[4].Get<int32>();
                accountMounts[i].RequiredSkillRank = fields[5].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} mount templates", i);
        }

        void LoadFactionSpecificMounts()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
                                                     "account_bound_mount_template abt ON pfs.alliance_id = abt.spellid WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 faction-specific mount templates");
                return;
            }

            factionSpecificAccountMounts.clear();

            int i = 0;
            do
            {
                Field* fields                       = result->Fetch();
                factionSpecificAccountMounts.push_back(FactionSpecificAccountMounts());
                factionSpecificAccountMounts[i].AllianceId = fields[0].Get<int32>();
                factionSpecificAccountMounts[i].HordeId    = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} faction-specific mount templates", i);
        }

        void LoadCompanions()
        {
            QueryResult result = WorldDatabase.Query("SELECT spellid, allowablerace FROM account_bound_companion_template");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 companion templates");
                return;
            }

            accountCompanions.clear();

            int i = 0;
            do
            {
                Field* fields               = result->Fetch();
                accountCompanions.push_back(AccountCompanions());
                accountCompanions[i].SpellId       = fields[0].Get<int32>();
                accountCompanions[i].AllowableRace = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} companion templates", i);
        }

        void LoadFactionSpecificCompanions()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
                                                     "account_bound_companion_template abt ON pfs.alliance_id = abt.spellid WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 faction-specific companion templates");
                return;
            }

            factionSpecificAccountCompanions.clear();

            int i = 0;
            do
            {
                Field* fields                           = result->Fetch();
                factionSpecificAccountCompanions.push_back(FactionSpecificAccountCompanions());
                factionSpecificAccountCompanions[i].AllianceId = fields[0].Get<int32>();
                factionSpecificAccountCompanions[i].HordeId    = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} faction-specific companion templates", i);
        }
};

void AddSC_mod_accountbound()
{
    new AccountBoundMounts();
    new AccountBoundCompanions();
    new AccountBoundData();
}
