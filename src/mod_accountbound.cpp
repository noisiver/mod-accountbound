#include "Player.h"
#include "ScriptMgr.h"

class AccountBound : public PlayerScript
{
    public:
        AccountBound() : PlayerScript("AccountBound") {}

        void OnLogin(Player* player) override
        {
            LoadMounts(player);
        }

    private:
        void LoadMounts(Player* player)
        {
            QueryResult result = CharacterDatabase.Query("SELECT DISTINCT `spell` FROM `character_spell` WHERE `guid` IN (SELECT guid FROM characters WHERE `account` = {}) AND `guid` <> {}", player->GetSession()->GetAccountId(), player->GetGUID().GetCounter());

            if (!result)
                return;

            do
            {
                Field* fields           = result->Fetch();
                uint32 spellId          = fields[0].Get<uint32>();
                const SpellEntry* spell = sSpellStore.LookupEntry(spellId);

                if (!player->HasSpell(spell->Id) && IsMountValid(player, spell))
                    player->learnSpell(spell->Id);
            } while (result->NextRow());
        }

        bool IsMountValid(Player* player, const SpellEntry* spell)
        {
            if (spell->Effect[0] == SPELL_EFFECT_APPLY_AURA && spell->EffectApplyAuraName[0] == SPELL_AURA_MOUNTED)
            {
                if (((spell->AttributesEx7 & SPELL_ATTR7_ALLIANCE_SPECIFIC_SPELL) && player->GetTeamId() != TEAM_ALLIANCE) ||
                    ((spell->AttributesEx7 & SPELL_ATTR7_HORDE_SPECIFIC_SPELL) && player->GetTeamId() != TEAM_HORDE) ||
                    (spell->SpellFamilyName == SPELLFAMILY_WARLOCK && player->getClass() != CLASS_WARLOCK) || 
                    (spell->SpellFamilyName == SPELLFAMILY_PALADIN && player->getClass() != CLASS_PALADIN) ||
                    (spell->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT && player->getClass() != CLASS_DEATH_KNIGHT))
                    return false;

                QueryResult result = WorldDatabase.Query("SELECT `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank` FROM `item_template` WHERE `spellid_2` = {}", spell->Id);

                if (!result)
                    return false;

                Field* fields            = result->Fetch();
                uint32 requiredLevel     = fields[0].Get<uint32>();
                uint32 requiredSkill     = fields[1].Get<uint32>();
                uint32 requiredSkillRank = fields[2].Get<uint32>();

                if (player->getLevel() < requiredLevel ||
                    (!player->HasSkill(requiredSkill) || player->GetSkillValue(requiredSkill) < requiredSkillRank))
                    return false;

                return true;
            }

            return false;
        }
};

void AddSC_mod_accountbound()
{
    new AccountBound();
}
