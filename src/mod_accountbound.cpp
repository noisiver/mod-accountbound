#include "Player.h"
#include "ScriptMgr.h"

struct Mounts
{
    uint32 SpellId;
    uint32 AllowableRace;
    uint32 AllowableClass;
    uint32 RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
};

struct FactionSpecificMounts
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<Mounts> mounts;
std::vector<FactionSpecificMounts> factionSpecificMounts;

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
            for (auto& accountBoundMount : mounts)
            {
                if (accountBoundMount.SpellId != spellID)
                    continue;

                int factionSpecificMountSpellId = FindFactionSpecificMount(accountBoundMount.SpellId);
                if (factionSpecificMountSpellId == -1)
                {
                    LoginDatabase.DirectExecute("REPLACE INTO account_bound_mounts (accountid, spellid, allowablerace, allowableclass, requiredlevel, "
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

                LoginDatabase.DirectExecute("REPLACE INTO account_bound_mounts (accountid, spellid, allowablerace, allowableclass, requiredlevel, requiredskill, "
                    "requiredskillrank) VALUES ({}, {}, {}, {}, {}, {}, {}), ({}, {}, {}, {}, {}, {}, {})",
                    player->GetSession()->GetAccountId(),
                    factionSpecificMounts[factionSpecificMountSpellId].AllianceId,
                    RACEMASK_ALLIANCE,
                    accountBoundMount.AllowableClass,
                    accountBoundMount.RequiredLevel,
                    accountBoundMount.RequiredSkill,
                    accountBoundMount.RequiredSkillRank,
                    player->GetSession()->GetAccountId(),
                    factionSpecificMounts[factionSpecificMountSpellId].HordeId,
                    RACEMASK_HORDE,
                    accountBoundMount.AllowableClass,
                    accountBoundMount.RequiredLevel,
                    accountBoundMount.RequiredSkill,
                    accountBoundMount.RequiredSkillRank);
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
            for (size_t i = 0; i != factionSpecificMounts.size(); ++i)
            {
                if (factionSpecificMounts[i].AllianceId == spellId || factionSpecificMounts[i].HordeId == spellId)
                    return i;
            }

            return -1;
        }
};

struct Companions
{
    uint32 SpellId;
    uint32 AllowableRace;
};

struct FactionSpecificCompanions
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<Companions> companions;
std::vector<FactionSpecificCompanions> factionSpecificCompanions;

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
            for (auto& accountBoundCompanion : companions)
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
                                            factionSpecificCompanions[factionCompanionSpellId].AllianceId,
                                            RACEMASK_ALLIANCE,
                                            player->GetSession()->GetAccountId(),
                                            factionSpecificCompanions[factionCompanionSpellId].HordeId,
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
            for (size_t i = 0; i != factionSpecificCompanions.size(); ++i)
            {
                if (factionSpecificCompanions[i].AllianceId == spellId || factionSpecificCompanions[i].HordeId == spellId)
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

            mounts.clear();

            int i = 0;
            do
            {
                Field* fields               = result->Fetch();
                mounts.push_back(Mounts());
                mounts[i].SpellId           = fields[0].Get<int32>();
                mounts[i].AllowableRace     = fields[1].Get<int32>();
                mounts[i].AllowableClass    = fields[2].Get<int32>();
                mounts[i].RequiredLevel     = fields[3].Get<int32>();
                mounts[i].RequiredSkill     = fields[4].Get<int32>();
                mounts[i].RequiredSkillRank = fields[5].Get<int32>();

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

            factionSpecificMounts.clear();

            int i = 0;
            do
            {
                Field* fields                       = result->Fetch();
                factionSpecificMounts.push_back(FactionSpecificMounts());
                factionSpecificMounts[i].AllianceId = fields[0].Get<int32>();
                factionSpecificMounts[i].HordeId    = fields[1].Get<int32>();

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

            companions.clear();

            int i = 0;
            do
            {
                Field* fields               = result->Fetch();
                companions.push_back(Companions());
                companions[i].SpellId       = fields[0].Get<int32>();
                companions[i].AllowableRace = fields[1].Get<int32>();

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

            factionSpecificCompanions.clear();

            int i = 0;
            do
            {
                Field* fields                           = result->Fetch();
                factionSpecificCompanions.push_back(FactionSpecificCompanions());
                factionSpecificCompanions[i].AllianceId = fields[0].Get<int32>();
                factionSpecificCompanions[i].HordeId    = fields[1].Get<int32>();

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
