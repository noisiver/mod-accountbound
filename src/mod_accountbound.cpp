#include "Player.h"
#include "ScriptMgr.h"

struct AccountAchievements
{
    uint32 Id;
    uint32 AllowableRace;
};

struct FactionSpecificAccountAchievements
{
    uint32 AllianceId;
    uint32 HordeId;
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

std::vector<AccountAchievements> accountAchievements;
std::vector<FactionSpecificAccountAchievements> factionSpecificAccountAchievements;
std::vector<AccountCompanions> accountCompanions;
std::vector<FactionSpecificAccountCompanions> factionSpecificAccountCompanions;
std::vector<AccountMounts> accountMounts;
std::vector<FactionSpecificAccountMounts> factionSpecificAccountMounts;

class AccountBoundAchievements : public PlayerScript
{
public:
    AccountBoundAchievements() : PlayerScript("AccountBoundAchievements") {}

    void OnAchiComplete(Player* player, AchievementEntry const* achievement) override
    {
        if (!player->IsGameMaster())
            SaveAchievements(player, achievement->ID);
    }

    void OnLogin(Player* player) override
    {
        if (!player->IsGameMaster())
            LoadAchievements(player);
    }

private:
    int FindFactionSpecificAchievement(uint32 achievementId)
    {
        for (size_t i = 0; i != factionSpecificAccountAchievements.size(); ++i)
        {
            if (factionSpecificAccountAchievements[i].AllianceId == achievementId || factionSpecificAccountAchievements[i].HordeId == achievementId)
                return i;
        }

        return -1;
    }

    void LoadAchievements(Player* player)
    {
        QueryResult result = LoginDatabase.Query("SELECT achievement FROM account_bound_achievements WHERE accountid={} AND allowablerace & {}",
            player->GetSession()->GetAccountId(),
            player->getRaceMask());

        if (!result)
            return;

        do
        {
            Field* fields = result->Fetch();
            uint32 achievementId = fields[0].Get<uint32>();

            if (!player->HasAchieved(achievementId))
                player->CompletedAchievement(sAchievementMgr->GetAchievement(achievementId));

        } while (result->NextRow());
    }

    void SaveAchievements(Player* player, uint32 achievementId)
    {
        for (auto& accountAchievement : accountAchievements)
        {
            if (accountAchievement.Id != achievementId)
                continue;

            int factionSpecificAchievementId = FindFactionSpecificAchievement(accountAchievement.Id);
            if (factionSpecificAchievementId == -1)
            {
                LoginDatabase.DirectExecute("REPLACE INTO account_bound_achievements (accountid, achievement, allowablerace)"
                    "VALUES ({}, {}, {})",
                    player->GetSession()->GetAccountId(),
                    achievementId,
                    accountAchievement.AllowableRace);
                continue;
            }

            LoginDatabase.DirectExecute("REPLACE INTO account_bound_achievements (accountid, achievement, allowablerace) "
                "VALUES ({}, {}, {}), ({}, {}, {})",
                player->GetSession()->GetAccountId(),
                factionSpecificAccountAchievements[factionSpecificAchievementId].AllianceId,
                RACEMASK_ALLIANCE,
                player->GetSession()->GetAccountId(),
                factionSpecificAccountAchievements[factionSpecificAchievementId].HordeId,
                RACEMASK_HORDE);
        }
    }
};

class AccountBoundCompanions : public PlayerScript
{
public:
    AccountBoundCompanions() : PlayerScript("AccountBoundCompanions") {}

    void OnLearnSpell(Player* player, uint32 spellID) override
    {
        if (!player->IsGameMaster())
            SaveCompanions(player, spellID);
    }

    void OnLogin(Player* player) override
    {
        if (!player->IsGameMaster())
            LoadCompanions(player);
    }

private:
    int FindFactionSpecificCompanion(uint32 spellId)
    {
        for (size_t i = 0; i != factionSpecificAccountCompanions.size(); ++i)
        {
            if (factionSpecificAccountCompanions[i].AllianceId == spellId || factionSpecificAccountCompanions[i].HordeId == spellId)
                return i;
        }

        return -1;
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
            Field* fields = result->Fetch();
            uint32 spellId = fields[0].Get<uint32>();

            if (!player->HasSpell(spellId))
                player->learnSpell(spellId);

        } while (result->NextRow());
    }

    void SaveCompanions(Player* player, uint32 spellID)
    {
        for (auto& accountCompanion : accountCompanions)
        {
            if (accountCompanion.SpellId != spellID)
                continue;

            int factionCompanionSpellId = FindFactionSpecificCompanion(accountCompanion.SpellId);
            if (factionCompanionSpellId == -1)
            {
                LoginDatabase.DirectExecute("REPLACE INTO account_bound_companions (accountid, spellid, allowablerace)"
                    "VALUES ({}, {}, {})",
                    player->GetSession()->GetAccountId(),
                    spellID,
                    accountCompanion.AllowableRace);
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
};

class AccountBoundData : public WorldScript
{
public:
    AccountBoundData() : WorldScript("AccountBoundData") {}

    void OnStartup() override
    {
        LOG_INFO("server.loading", "Loading account bound templates...");
        LoadAchievements();
        LoadFactionSpecificAchievements();
        LoadCompanions();
        LoadFactionSpecificCompanions();
        LoadMounts();
        LoadFactionSpecificMounts();
    }

private:
    void LoadAchievements()
    {
        QueryResult result = WorldDatabase.Query("SELECT id, allowablerace FROM account_bound_achievement_template");

        if (!result)
        {
            LOG_INFO("server.loading", ">> Loaded 0 achievement templates");
            return;
        }

        accountAchievements.clear();

        int i = 0;
        do
        {
            accountAchievements.push_back(AccountAchievements());
            Field* fields = result->Fetch();
            accountAchievements[i].Id = fields[0].Get<int32>();
            accountAchievements[i].AllowableRace = fields[1].Get<int32>();

            i++;
        } while (result->NextRow());

        LOG_INFO("server.loading", ">> Loaded {} achievement templates", i);
    }

    void LoadFactionSpecificAchievements()
    {
        QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells pfs LEFT OUTER JOIN "
            "account_bound_achievement_template abt ON pfs.alliance_id = abt.id WHERE abt.allowablerace = 1101");

        if (!result)
        {
            LOG_INFO("server.loading", ">> Loaded 0 faction-specific achievement templates");
            return;
        }

        factionSpecificAccountAchievements.clear();

        int i = 0;
        do
        {
            factionSpecificAccountAchievements.push_back(FactionSpecificAccountAchievements());
            Field* fields = result->Fetch();
            factionSpecificAccountAchievements[i].AllianceId = fields[0].Get<int32>();
            factionSpecificAccountAchievements[i].HordeId = fields[1].Get<int32>();

            i++;
        } while (result->NextRow());

        LOG_INFO("server.loading", ">> Loaded {} faction-specific achievement templates", i);
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
            accountCompanions.push_back(AccountCompanions());
            Field* fields = result->Fetch();
            accountCompanions[i].SpellId = fields[0].Get<int32>();
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
            factionSpecificAccountCompanions.push_back(FactionSpecificAccountCompanions());
            Field* fields = result->Fetch();
            factionSpecificAccountCompanions[i].AllianceId = fields[0].Get<int32>();
            factionSpecificAccountCompanions[i].HordeId = fields[1].Get<int32>();

            i++;
        } while (result->NextRow());

        LOG_INFO("server.loading", ">> Loaded {} faction-specific companion templates", i);
    }

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
            accountMounts.push_back(AccountMounts());
            Field* fields = result->Fetch();
            accountMounts[i].SpellId = fields[0].Get<int32>();
            accountMounts[i].AllowableRace = fields[1].Get<int32>();
            accountMounts[i].AllowableClass = fields[2].Get<int32>();
            accountMounts[i].RequiredLevel = fields[3].Get<int32>();
            accountMounts[i].RequiredSkill = fields[4].Get<int32>();
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
            factionSpecificAccountMounts.push_back(FactionSpecificAccountMounts());
            Field* fields = result->Fetch();
            factionSpecificAccountMounts[i].AllianceId = fields[0].Get<int32>();
            factionSpecificAccountMounts[i].HordeId = fields[1].Get<int32>();

            i++;
        } while (result->NextRow());

        LOG_INFO("server.loading", ">> Loaded {} faction-specific mount templates", i);
    }
};

enum RidingAchievement
{
    ACHIEVEMENT_APPRENTICE = 891,
    ACHIEVEMENT_JOURNEYMAN = 889,
    ACHIEVEMENT_EXPERT     = 890,
    ACHIEVEMENT_ARTISAN    = 892
};

class AccountBoundMounts : public PlayerScript
{
public:
    AccountBoundMounts() : PlayerScript("AccountBoundMounts") {}

    void OnAchiComplete(Player* player, AchievementEntry const* achievement) override
    {
        if (achievement->ID == ACHIEVEMENT_APPRENTICE || achievement->ID == ACHIEVEMENT_JOURNEYMAN || achievement->ID == ACHIEVEMENT_EXPERT || achievement->ID == ACHIEVEMENT_ARTISAN)
            if (!player->IsGameMaster())
                LearnMounts(player);
    }

    void OnLearnSpell(Player* player, uint32 spellID) override
    {
        if (!player->IsGameMaster())
            SaveMounts(player, spellID);
    }

    void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
    {
        if (!player->IsGameMaster())
            LearnMounts(player);
    }

    void OnLogin(Player* player) override
    {
        if (!player->IsGameMaster())
            LearnMounts(player);
    }

private:
    int FindFactionSpecificMount(uint32 spellId)
    {
        for (size_t i = 0; i != factionSpecificAccountMounts.size(); ++i)
        {
            if (factionSpecificAccountMounts[i].AllianceId == spellId || factionSpecificAccountMounts[i].HordeId == spellId)
                return i;
        }

        return -1;
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
            Field* fields = result->Fetch();
            uint32 spellId = fields[0].Get<uint32>();

            if (!player->HasSpell(spellId))
                player->learnSpell(spellId);

        } while (result->NextRow());
    }

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
};

void AddSC_mod_accountbound()
{
    new AccountBoundAchievements();
    new AccountBoundCompanions();
    new AccountBoundData();
    new AccountBoundMounts();
}
