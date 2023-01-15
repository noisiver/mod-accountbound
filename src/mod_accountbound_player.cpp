#include "Player.h"

#include "mod_accountbound.h"

void AccountBound::OnAchiComplete(Player* player, AchievementEntry const* achievement)
{
    if (player->IsGameMaster() && !EnableGamemasters)
        return;

    if (achievement->ID == ACHIEVEMENT_APPRENTICE || achievement->ID == ACHIEVEMENT_JOURNEYMAN || achievement->ID == ACHIEVEMENT_EXPERT || achievement->ID == ACHIEVEMENT_ARTISAN)
        LearnMounts(player);
}

void AccountBound::OnAfterStoreOrEquipNewItem(Player* player, uint32 /*vendorslot*/, Item* item, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* /*pVendor*/, VendorItem const* /*crItem*/, bool /*bStore*/)
{
    if (player->IsGameMaster() && !EnableGamemasters)
        return;

    if (!item || !item->GetTemplate() || !EnableAccountHeirlooms)
        return;

    if (item->GetTemplate()->Quality == ITEM_QUALITY_HEIRLOOM && item->GetEntry() != 49177)
    {
        SaveHeirloom(player, item->GetEntry());
        item->SetNotRefundable(player);
    }
}

void AccountBound::OnLearnSpell(Player* player, uint32 spellID)
{
    if (player->IsGameMaster() && !EnableGamemasters)
        return;

    SaveCompanion(player, spellID);
    SaveMount(player, spellID);
}

void AccountBound::OnLevelChanged(Player* player, uint8 /*oldlevel*/)
{
    if (player->IsGameMaster() && !EnableGamemasters)
        return;

    LearnMounts(player);
}

void AccountBound::OnLogin(Player* player)
{
    if (player->IsGameMaster() && !EnableGamemasters)
        return;

    LearnCompanions(player);
    LearnMounts(player);
}

void AccountBound::SaveCompanion(Player* player, uint32 spell_id)
{
    if (!EnableAccountCompanions)
        return;

    for (auto& accountCompanion : accountCompanions)
    {
        if (accountCompanion.SpellId != spell_id)
            continue;

        int factionCompanionSpellId = FindFactionSpecificCompanion(accountCompanion.SpellId);
        if (factionCompanionSpellId == -1)
        {
            LoginDatabase.Execute("REPLACE INTO `account_bound_companions` (`account_id`, `spell_id`, `allowable_race`)"
                "VALUES ({}, {}, {})",
                player->GetSession()->GetAccountId(),
                spell_id,
                accountCompanion.AllowableRace);
            continue;
        }

        LoginDatabase.Execute("REPLACE INTO `account_bound_companions` (`account_id`, `spell_id`, `allowable_race`) "
            "VALUES ({}, {}, {}), ({}, {}, {})",
            player->GetSession()->GetAccountId(),
            factionSpecificAccountCompanions[factionCompanionSpellId].AllianceId,
            RACEMASK_ALLIANCE,
            player->GetSession()->GetAccountId(),
            factionSpecificAccountCompanions[factionCompanionSpellId].HordeId,
            RACEMASK_HORDE);
    }
}

void AccountBound::LearnCompanions(Player* player)
{
    if (!EnableAccountCompanions)
        return;

    QueryResult result = LoginDatabase.Query("SELECT `spell_id` FROM `account_bound_companions` WHERE `account_id`={} AND `allowable_race` & {}",
        player->GetSession()->GetAccountId(),
        player->getRaceMask());

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 spell_id = fields[0].Get<uint32>();

        if (!player->HasSpell(spell_id))
            player->learnSpell(spell_id);

    } while (result->NextRow());
}

void AccountBound::SaveHeirloom(Player* player, uint32 item_id)
{
    LoginDatabase.Execute("REPLACE INTO `account_bound_heirlooms` (`account_id`, `item_id`) "
        "VALUES ({}, {})",
        player->GetSession()->GetAccountId(),
        item_id);
}

void AccountBound::LearnMounts(Player* player)
{
    if (!EnableAccountMounts)
        return;

    QueryResult result = LoginDatabase.Query("SELECT `spell_id` FROM `account_bound_mounts` WHERE `account_id` = {} AND `allowable_race` & {} "
            "AND `allowable_class` & {} AND `required_level` <= {} AND (`required_skill` = 0 OR `required_skill_rank` <= {})",
            player->GetSession()->GetAccountId(),
            player->getRaceMask(),
            player->getClassMask(),
            player->GetLevel(),
            player->GetSkillValue(SKILL_RIDING));

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 spell_id = fields[0].Get<uint32>();

        if (!player->HasSpell(spell_id))
            player->learnSpell(spell_id);

    } while (result->NextRow());
}

void AccountBound::SaveMount(Player* player, uint32 spell_id)
{
    if (!EnableAccountMounts)
        return;

    for (auto& mount : accountMounts)
    {
        if (mount.SpellId != spell_id)
            continue;

        int factionSpecificMountSpellId = FindFactionSpecificMount(mount.SpellId);
        if (factionSpecificMountSpellId == -1)
        {
            LoginDatabase.Execute("REPLACE INTO `account_bound_mounts` (`account_id`, `spell_id`, `allowable_race`, `allowable_class`, `required_level`, "
                "`required_skill`, `required_skill_rank`) VALUES ({}, {}, {}, {}, {}, {}, {})",
                player->GetSession()->GetAccountId(),
                spell_id,
                mount.AllowableRace,
                mount.AllowableClass,
                mount.RequiredLevel,
                mount.RequiredSkill,
                mount.RequiredSkillRank);
            continue;
        }

        LoginDatabase.Execute("REPLACE INTO `account_bound_mounts` (`account_id`, `spell_id`, `allowable_race`, `allowable_class`, `required_level`, `required_skill`, "
            "`required_skill_rank`) VALUES ({}, {}, {}, {}, {}, {}, {}), ({}, {}, {}, {}, {}, {}, {})",
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
