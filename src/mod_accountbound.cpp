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

struct AccountBoundCompanions
{
    uint32 SpellId;
    uint32 AllowableRace;
};

struct FactionChangeCompanions
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<AccountBoundCompanions> accountBoundCompanions;
std::vector<FactionChangeCompanions> factionChangeCompanions;

struct AccountBoundAchievements
{
    uint32 AchievementId;
    uint32 AllowableRace;
};

struct FactionChangeAchievements
{
    uint32 AllianceId;
    uint32 HordeId;
};

std::vector<AccountBoundAchievements> accountBoundAchievements;
std::vector<FactionChangeAchievements> factionChangeAchievements;

class AccountBoundPlayer : public PlayerScript
{
    public:
        AccountBoundPlayer() : PlayerScript("AccountBound") {}

        void OnLearnSpell(Player* player, uint32 spellID) override
        {
            SaveAccountBoundMounts(player, spellID);
            SaveAccountBoundCompanions(player, spellID);
        }

        void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
        {
            LearnAccountBoundMounts(player);
        }

        void OnLogin(Player* player) override
        {
            LearnAccountBoundMounts(player);
            LearnAccountBoundCompanions(player);
            LearnAccountBoundAchievements(player);
        }

        void OnAchiSave(CharacterDatabaseTransaction /*trans*/, Player* player, uint16 achId, CompletedAchievementData /*achiData*/) override
        {
            SaveAccountBoundAchievements(player, achId);
        }
    private:
        void SaveAccountBoundMounts(Player* player, uint32 spellID)
        {
            for (auto& accountBoundMount : accountBoundMounts)
            {
                if (accountBoundMount.SpellId != spellID)
                    continue;

                int factionMountSpellId = GetFactionChangeMount(accountBoundMount.SpellId);
                if (factionMountSpellId == -1)
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

        void LearnAccountBoundMounts(Player* player)
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

        int GetFactionChangeMount(uint32 spellId)
        {
            for (size_t i = 0; i != factionChangeMounts.size(); ++i)
            {
                if (factionChangeMounts[i].AllianceId == spellId || factionChangeMounts[i].HordeId == spellId)
                    return i;
            }

            return -1;
        }

        void SaveAccountBoundCompanions(Player* player, uint32 spellID)
        {
            for (auto& accountBoundCompanion : accountBoundCompanions)
            {
                if (accountBoundCompanion.SpellId != spellID)
                    continue;

                int factionCompanionSpellId = GetFactionChangeCompanion(accountBoundCompanion.SpellId);
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
                                            factionChangeCompanions[factionCompanionSpellId].AllianceId,
                                            RACEMASK_ALLIANCE,
                                            player->GetSession()->GetAccountId(),
                                            factionChangeCompanions[factionCompanionSpellId].HordeId,
                                            RACEMASK_HORDE);
            }
        }

        void LearnAccountBoundCompanions(Player* player)
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

        int GetFactionChangeCompanion(uint32 spellId)
        {
            for (size_t i = 0; i != factionChangeCompanions.size(); ++i)
            {
                if (factionChangeCompanions[i].AllianceId == spellId || factionChangeCompanions[i].HordeId == spellId)
                    return i;
            }

            return -1;
        }

        void SaveAccountBoundAchievements(Player* player, uint16 achievement)
        {
            for (auto& accountBoundAchievement : accountBoundAchievements)
            {
                if (accountBoundAchievement.AchievementId != achievement)
                    continue;

                int factionAchievementId = GetFactionChangeAchievement(accountBoundAchievement.AchievementId);
                if (factionAchievementId == -1)
                {
                    LoginDatabase.DirectExecute("REPLACE INTO account_bound_achievements (accountid, achievement, allowablerace)"
                                                "VALUES ({}, {}, {})",
                                                player->GetSession()->GetAccountId(),
                                                achievement,
                                                accountBoundAchievement.AllowableRace);
                    continue;
                }

                LoginDatabase.DirectExecute("REPLACE INTO account_bound_achievements (accountid, achievement, allowablerace) "
                    "VALUES ({}, {}, {}), ({}, {}, {})",
                    player->GetSession()->GetAccountId(),
                    factionChangeAchievements[factionAchievementId].AllianceId,
                    RACEMASK_ALLIANCE,
                    player->GetSession()->GetAccountId(),
                    factionChangeAchievements[factionAchievementId].HordeId,
                    RACEMASK_HORDE);
            }
        }

        void LearnAccountBoundAchievements(Player* player)
        {
            QueryResult result = LoginDatabase.Query("SELECT achievement FROM account_bound_achievements WHERE accountid={} AND allowablerace & {}",
                                                     player->GetSession()->GetAccountId(),
                                                     player->getRaceMask());

            if (!result)
                return;

            do
            {
                Field* fields          = result->Fetch();
                uint32 achievementId   = fields[0].Get<uint32>();

                player->CompletedAchievement(sAchievementMgr->GetAchievement(achievementId));
            } while (result->NextRow());
        }

        int GetFactionChangeAchievement(uint32 achievementId)
        {
            for (size_t i = 0; i != factionChangeAchievements.size(); ++i)
            {
                if (factionChangeAchievements[i].AllianceId == achievementId || factionChangeAchievements[i].HordeId == achievementId)
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
            LoadAccountBoundCompanions();
            LoadAccountBoundFactionChangeCompanions();
            LoadAccountBoundAchievements();
            LoadAccountBoundFactionChangeAchievements();
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

            LOG_INFO("server.loading", ">> Loaded {} account bound mount templates", i);
        }

        void LoadAccountBoundFactionChangeMounts()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
                                                     "account_bound_mount_template abt ON pfs.alliance_id = abt.spellid WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound faction change mount templates");
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

            LOG_INFO("server.loading", ">> Loaded {} account bound faction change mount templates", i);
        }

        void LoadAccountBoundCompanions()
        {
            QueryResult result = WorldDatabase.Query("SELECT spellid, allowablerace FROM account_bound_companion_template");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound companion templates");
                return;
            }

            accountBoundCompanions.clear();

            int i = 0;
            do
            {
                Field* fields                           = result->Fetch();
                accountBoundCompanions.push_back(AccountBoundCompanions());
                accountBoundCompanions[i].SpellId       = fields[0].Get<int32>();
                accountBoundCompanions[i].AllowableRace = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound companion templates", i);
        }

        void LoadAccountBoundFactionChangeCompanions()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
                                                     "account_bound_companion_template abt ON pfs.alliance_id = abt.spellid WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound faction change companion templates");
                return;
            }

            factionChangeCompanions.clear();

            int i = 0;
            do
            {
                Field* fields                         = result->Fetch();
                factionChangeCompanions.push_back(FactionChangeCompanions());
                factionChangeCompanions[i].AllianceId = fields[0].Get<int32>();
                factionChangeCompanions[i].HordeId    = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound faction change companion templates", i);
        }

        void LoadAccountBoundAchievements()
        {
            QueryResult result = WorldDatabase.Query("SELECT id, allowablerace FROM account_bound_achievement_template");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound achievement templates");
                return;
            }

            accountBoundAchievements.clear();

            int i = 0;
            do
            {
                Field* fields                             = result->Fetch();
                accountBoundAchievements.push_back(AccountBoundAchievements());
                accountBoundAchievements[i].AchievementId = fields[0].Get<int32>();
                accountBoundAchievements[i].AllowableRace = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound achievement templates", i);
        }

        void LoadAccountBoundFactionChangeAchievements()
        {
            QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_achievement pfa LEFT OUTER JOIN "
                                                     "account_bound_achievement_template abt ON pfa.alliance_id = abt.id WHERE abt.allowablerace = 1101");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 account bound faction change achievement templates");
                return;
            }

            factionChangeAchievements.clear();

            int i = 0;
            do
            {
                Field* fields                           = result->Fetch();
                factionChangeAchievements.push_back(FactionChangeAchievements());
                factionChangeAchievements[i].AllianceId = fields[0].Get<int32>();
                factionChangeAchievements[i].HordeId    = fields[1].Get<int32>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} account bound faction change achievement templates", i);
        }
};

void AddSC_mod_accountbound()
{
    new AccountBoundPlayer();
    new AccountBoundWorld();
}
