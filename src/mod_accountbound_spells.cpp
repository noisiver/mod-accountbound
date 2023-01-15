#include "mod_accountbound.h"

void AccountBound::LoadCompanions()
{
    QueryResult result = LoginDatabase.Query("SELECT `spell_id`, `allowable_race` FROM `account_bound_companion_template`");

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

void AccountBound::LoadFactionSpecificCompanions()
{
    QueryResult result = LoginDatabase.Query("SELECT `alliance_id`, `horde_id` FROM `account_bound_factionchange_spells` pfs LEFT OUTER JOIN "
        "`account_bound_companion_template` abt ON pfs.`alliance_id` = abt.`spell_id` WHERE abt.`allowable_race` = 1101");

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

int AccountBound::FindFactionSpecificCompanion(uint32 spell_id)
{
    for (size_t i = 0; i != factionSpecificAccountCompanions.size(); ++i)
    {
        if (factionSpecificAccountCompanions[i].AllianceId == spell_id || factionSpecificAccountCompanions[i].HordeId == spell_id)
            return i;
    }

    return -1;
}

void AccountBound::LoadMounts()
{
    QueryResult result = LoginDatabase.Query("SELECT `spell_id`, `allowable_race`, `allowable_class`, `required_level`, "
        "`required_skill`, `required_skill_rank` FROM `account_bound_mount_template`");

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

void AccountBound::LoadFactionSpecificMounts()
{
    QueryResult result = LoginDatabase.Query("SELECT `alliance_id`, `horde_id` FROM `account_bound_factionchange_spells` pfs LEFT OUTER JOIN "
        "`account_bound_mount_template` abt ON pfs.`alliance_id` = abt.`spell_id` WHERE abt.`allowable_race` = 1101");

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

int AccountBound::FindFactionSpecificMount(uint32 spell_id)
{
    for (size_t i = 0; i != factionSpecificAccountMounts.size(); ++i)
    {
        if (factionSpecificAccountMounts[i].AllianceId == spell_id || factionSpecificAccountMounts[i].HordeId == spell_id)
            return i;
    }

    return -1;
}
