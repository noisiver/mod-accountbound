#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

bool abEnableGamemasters;
bool abEnableAccountCompanions;
bool abEnableAccountHeirlooms;
bool abEnableAccountMounts;
bool abEnableLinkedAccounts;

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
        if (abEnableAccountCompanions)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || abEnableGamemasters)
                SaveCompanions(player, spellID);
    }

    void OnLogin(Player* player) override
    {
        if (abEnableAccountCompanions)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || abEnableGamemasters)
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
        QueryResult result;

        if (abEnableLinkedAccounts)
        {
            result = LoginDatabase.Query("SELECT DISTINCT spell_id FROM account_bound_companions abc "
                "LEFT OUTER JOIN account_bound_linked_accounts abls ON(abc.account_id = abls.account_id OR abc.account_id = abls.linked_account_id) "
                "WHERE(abc.account_id = {} OR abls.account_id = {} OR abls.linked_account_id = {}) AND allowable_race & {}",
                player->GetSession()->GetAccountId(),
                player->GetSession()->GetAccountId(),
                player->GetSession()->GetAccountId(),
                player->getRaceMask());
        }
        else
        {
            result = LoginDatabase.Query("SELECT spell_id FROM account_bound_companions WHERE account_id={} AND allowable_race & {}",
                player->GetSession()->GetAccountId(),
                player->getRaceMask());
        }

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
        abEnableGamemasters = sConfigMgr->GetOption<bool>("AccountBound.Gamemasters", 0);
        abEnableAccountCompanions = sConfigMgr->GetOption<bool>("AccountBound.Companions", 1);
        abEnableAccountHeirlooms = sConfigMgr->GetOption<bool>("AccountBound.Heirlooms", 0);
        abEnableAccountMounts = sConfigMgr->GetOption<bool>("AccountBound.Mounts", 1);
        abEnableLinkedAccounts = sConfigMgr->GetOption<bool>("AccountBound.LinkedAccounts", 0);
    }

    void OnStartup() override
    {
        LOG_INFO("server.loading", "Loading account bound templates...");

        if (abEnableAccountCompanions)
        {
            LoadCompanions();
            LoadFactionSpecificCompanions();
        }

        if (abEnableAccountMounts)
        {
            LoadMounts();
            LoadFactionSpecificMounts();
        }
    }

private:
    void LoadCompanions()
    {
        QueryResult result = LoginDatabase.Query("SELECT spell_id, allowable_race FROM account_bound_companion_template");

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
        QueryResult result = LoginDatabase.Query("SELECT alliance_id, horde_id FROM account_bound_factionchange_spells pfs LEFT OUTER JOIN "
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
        QueryResult result = LoginDatabase.Query("SELECT spell_id, allowable_race, allowable_class, required_level, "
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
        QueryResult result = LoginDatabase.Query("SELECT alliance_id, horde_id FROM account_bound_factionchange_spells pfs LEFT OUTER JOIN "
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

class AccountBoundHeirlooms : public PlayerScript
{
public:
    AccountBoundHeirlooms() : PlayerScript("AccountBoundHeirlooms") {}

    void OnAfterStoreOrEquipNewItem(Player* player, uint32 /*vendorslot*/, Item* item, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* /*pVendor*/, VendorItem const* /*crItem*/, bool /*bStore*/) override
    {
        if (!item || !item->GetTemplate() || !abEnableAccountHeirlooms)
            return;

        if (item->GetTemplate()->Quality == ITEM_QUALITY_HEIRLOOM && item->GetEntry() != 49177)
        {
            SaveHeirlooms(player, item->GetEntry());
            item->SetNotRefundable(player);
        }
    }

private:
    void SaveHeirlooms(Player* player, uint32 itemId)
    {
        LoginDatabase.DirectExecute("REPLACE INTO account_bound_heirlooms (account_id, item_id) "
            "VALUES ({}, {})",
            player->GetSession()->GetAccountId(),
            itemId);
    }
};

class AccountBoundHeirloomHoarder : CreatureScript
{
public:
    AccountBoundHeirloomHoarder() : CreatureScript("npc_account_bound_heirlooms") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (!abEnableAccountHeirlooms)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("This feature is currently disabled.");
            CloseGossipMenuFor(player);
            return true;
        }

        QueryResult result;

        if (abEnableLinkedAccounts)
        {
            QueryResult result = LoginDatabase.Query("SELECT DISTINCT item_id FROM account_bound_heirlooms abh "
                "LEFT OUTER JOIN account_bound_linked_accounts abls ON(abh.account_id = abls.account_id OR abh.account_id = abls.linked_account_id) "
                "WHERE(abm.account_id = {} OR abls.account_id = {} OR abls.linked_account_id = {})",
                player->GetSession()->GetAccountId(),
                player->GetSession()->GetAccountId(),
                player->GetSession()->GetAccountId());
        }
        else
        {
            QueryResult result = LoginDatabase.Query("SELECT item_id FROM account_bound_heirlooms WHERE account_id = {}",
                player->GetSession()->GetAccountId());
        }

        if (!result)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You don't have any stored heirlooms.");
            CloseGossipMenuFor(player);
            return true;
        }

        uint32 vendorId = 7500000 + player->GetSession()->GetAccountId();

        if (const VendorItemData* vendorData = sObjectMgr->GetNpcVendorItemList(vendorId))
        {
            uint8 vendorItemCount = vendorData->GetItemCount();
            for (int i = 0; i < vendorItemCount; i++)
            {
                sObjectMgr->RemoveVendorItem(vendorId, vendorData->GetItem(i)->item);
            }
        }

        do
        {
            Field* fields = result->Fetch();
            sObjectMgr->AddVendorItem(vendorId, fields[0].Get<uint32>(), 0, 0, 0, false);
        } while (result->NextRow());

        player->GetSession()->SendListInventory(creature->GetGUID(), vendorId);

        return true;
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
        if (abEnableAccountMounts)
            if (achievement->ID == ACHIEVEMENT_APPRENTICE || achievement->ID == ACHIEVEMENT_JOURNEYMAN || achievement->ID == ACHIEVEMENT_EXPERT || achievement->ID == ACHIEVEMENT_ARTISAN)
                if (player->GetSession()->GetSecurity() == SEC_PLAYER || abEnableGamemasters)
                    LearnMounts(player);
    }

    void OnLearnSpell(Player* player, uint32 spellID) override
    {
        if (abEnableAccountMounts)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || abEnableGamemasters)
                SaveMounts(player, spellID);
    }

    void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
    {
        if (abEnableAccountMounts)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || abEnableGamemasters)
                LearnMounts(player);
    }

    void OnLogin(Player* player) override
    {
        if (abEnableAccountMounts)
            if (player->GetSession()->GetSecurity() == SEC_PLAYER || abEnableGamemasters)
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
        QueryResult result;

        if (abEnableLinkedAccounts)
        {
            result = LoginDatabase.Query("SELECT DISTINCT spell_id FROM account_bound_mounts abm "
                "LEFT OUTER JOIN account_bound_linked_accounts abls ON(abm.account_id = abls.account_id OR abm.account_id = abls.linked_account_id) "
                "WHERE(abm.account_id = {} OR abls.account_id = {} OR abls.linked_account_id = {}) AND allowable_race & {} AND allowable_class & {} AND required_level <= {} AND(required_skill = 0 OR required_skill_rank <= {})",
                player->GetSession()->GetAccountId(),
                player->GetSession()->GetAccountId(),
                player->GetSession()->GetAccountId(),
                player->getRaceMask(),
                player->getClassMask(),
                player->getLevel(),
                player->GetSkillValue(SKILL_RIDING));
        }
        else
        {
            result = LoginDatabase.Query("SELECT spell_id FROM account_bound_mounts WHERE account_id = {} AND allowable_race & {} "
                "AND allowable_class & {} AND required_level <= {} AND (required_skill = 0 OR required_skill_rank <= {})",
                player->GetSession()->GetAccountId(),
                player->getRaceMask(),
                player->getClassMask(),
                player->getLevel(),
                player->GetSkillValue(SKILL_RIDING));
        }

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
    new AccountBoundHeirlooms();
    new AccountBoundHeirloomHoarder();
    new AccountBoundMounts();
}
