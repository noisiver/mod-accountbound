#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"

bool enableGamemasters;
bool enableAccountCompanions;
bool enableAccountMounts;

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

std::vector<AccountCompanions> accountCompanions;
std::vector<FactionSpecificAccountCompanions> factionSpecificAccountCompanions;
std::vector<AccountMounts> accountMounts;
std::vector<FactionSpecificAccountMounts> factionSpecificAccountMounts;

class AccountBoundCompanions : public PlayerScript
{
public:
    AccountBoundCompanions() : PlayerScript("AccountBoundCompanions") {}

    void OnLearnSpell(Player* player, uint32 spellID) override
    {
        if (enableAccountCompanions)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || enableGamemasters)
                SaveCompanions(player, spellID);
    }

    void OnLogin(Player* player) override
    {
        if (enableAccountCompanions)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || enableGamemasters)
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
        QueryResult result = LoginDatabase.Query("SELECT spell_id FROM account_bound_companions WHERE account_id={} AND allowable_race & {}",
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
                LoginDatabase.DirectExecute("REPLACE INTO account_bound_companions (account_id, spell_id, allowable_race)"
                    "VALUES ({}, {}, {})",
                    player->GetSession()->GetAccountId(),
                    spellID,
                    accountCompanion.AllowableRace);
                continue;
            }

            LoginDatabase.DirectExecute("REPLACE INTO account_bound_companions (account_id, spell_id, allowable_race) "
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

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        enableGamemasters = sConfigMgr->GetOption<bool>("AccountBound.Gamemasters", 0);
        enableAccountCompanions = sConfigMgr->GetOption<bool>("AccountBound.Companions", 1);
        enableAccountMounts = sConfigMgr->GetOption<bool>("AccountBound.Mounts", 1);
    }

    void OnStartup() override
    {
        LOG_INFO("server.loading", "Loading account bound templates...");

        if (enableAccountCompanions)
        {
            LoadCompanions();
            LoadFactionSpecificCompanions();
        }

        if (enableAccountMounts)
        {
            LoadMounts();
            LoadFactionSpecificMounts();
        }
    }

private:
    void LoadCompanions()
    {
        QueryResult result = WorldDatabase.Query("SELECT spell_id, allowable_race FROM account_bound_companion_template");

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
            "account_bound_companion_template abt ON pfs.alliance_id = abt.spell_id WHERE abt.allowable_race = 1101");

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
        QueryResult result = WorldDatabase.Query("SELECT spell_id, allowable_race, allowable_class, required_level, "
            "required_skill, required_skill_rank FROM account_bound_mount_template");

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
            "account_bound_mount_template abt ON pfs.alliance_id = abt.spell_id WHERE abt.allowable_race = 1101");

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
    ACHIEVEMENT_EXPERT = 890,
    ACHIEVEMENT_ARTISAN = 892
};

class AccountBoundMounts : public PlayerScript
{
public:
    AccountBoundMounts() : PlayerScript("AccountBoundMounts") {}

    void OnAchiComplete(Player* player, AchievementEntry const* achievement) override
    {
        if (enableAccountMounts)
            if (achievement->ID == ACHIEVEMENT_APPRENTICE || achievement->ID == ACHIEVEMENT_JOURNEYMAN || achievement->ID == ACHIEVEMENT_EXPERT || achievement->ID == ACHIEVEMENT_ARTISAN)
                if (player->GetSession()->GetSecurity() == SEC_PLAYER || enableGamemasters)
                    LearnMounts(player);
    }

    void OnLearnSpell(Player* player, uint32 spellID) override
    {
        if (enableAccountMounts)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || enableGamemasters)
                SaveMounts(player, spellID);
    }

    void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
    {
        if (enableAccountMounts)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || enableGamemasters)
                LearnMounts(player);
    }

    void OnLogin(Player* player) override
    {
        if (enableAccountMounts)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || enableGamemasters)
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
        QueryResult result = LoginDatabase.Query("SELECT spell_id FROM account_bound_mounts WHERE account_id={} AND allowable_race & {} "
            "AND allowable_class & {} AND required_level <= {} AND (required_skill = 0 OR required_skill_rank <= {})",
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
                LoginDatabase.DirectExecute("REPLACE INTO account_bound_mounts (account_id, spell_id, allowable_race, allowable_class, required_level, "
                    "required_skill, required_skill_rank) VALUES ({}, {}, {}, {}, {}, {}, {})",
                    player->GetSession()->GetAccountId(),
                    spellID,
                    mount.AllowableRace,
                    mount.AllowableClass,
                    mount.RequiredLevel,
                    mount.RequiredSkill,
                    mount.RequiredSkillRank);
                continue;
            }

            LoginDatabase.DirectExecute("REPLACE INTO account_bound_mounts (account_id, spell_id, allowable_race, allowable_class, required_level, required_skill, "
                "required_skill_rank) VALUES ({}, {}, {}, {}, {}, {}, {}), ({}, {}, {}, {}, {}, {}, {})",
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
    new AccountBoundCompanions();
    new AccountBoundData();
    new AccountBoundMounts();
}
