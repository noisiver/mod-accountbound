#ifndef MOD_ACCOUNTBOUND
#define MOD_ACCOUNTBOUND

#include "ScriptMgr.h"

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

enum RidingAchievement
{
    ACHIEVEMENT_APPRENTICE = 891,
    ACHIEVEMENT_JOURNEYMAN = 889,
    ACHIEVEMENT_EXPERT = 890,
    ACHIEVEMENT_ARTISAN = 892
};

class AccountBound : public CreatureScript, PlayerScript, WorldScript
{
public:
    AccountBound();

    // CreatureScript
    bool OnGossipHello(Player* /*player*/, Creature* /*creature*/) override;

    // PlayerScript
    void OnAchiComplete(Player* /*player*/, AchievementEntry const* /*achievement*/) override;
    void OnAfterStoreOrEquipNewItem(Player* /*player*/, uint32 /*vendorslot*/, Item* /*item*/, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* /*pVendor*/, VendorItem const* /*crItem*/, bool /*bStore*/) override;
    void OnLearnSpell(Player* /*player*/, uint32 /*spellID*/) override;
    void OnLevelChanged(Player* /*player*/, uint8 /*oldlevel*/) override;
    void OnLogin(Player* /*player*/) override;

    // WorldScript
    void OnAfterConfigLoad(bool /*reload*/) override;
    void OnStartup() override;

private:
    std::vector<AccountCompanions> accountCompanions;
    std::vector<FactionSpecificAccountCompanions> factionSpecificAccountCompanions;
    std::vector<AccountMounts> accountMounts;
    std::vector<FactionSpecificAccountMounts> factionSpecificAccountMounts;

    bool EnableGamemasters;
    bool EnableAccountCompanions;
    bool EnableAccountHeirlooms;
    bool EnableAccountMounts;

    void LoadCompanions();
    void LoadFactionSpecificCompanions();
    void SaveCompanion(Player* /*player*/, uint32 /*spell_id*/);
    void LearnCompanions(Player* /*player*/);
    int FindFactionSpecificCompanion(uint32 /*spell_id*/);

    void SaveHeirloom(Player* /*player*/, uint32 /*itemId*/);

    void LoadMounts();
    void LoadFactionSpecificMounts();
    void SaveMount(Player* /*player*/, uint32 /*spell_id*/);
    void LearnMounts(Player* /*player*/);
    int FindFactionSpecificMount(uint32 /*spell_id*/);
};

#endif
