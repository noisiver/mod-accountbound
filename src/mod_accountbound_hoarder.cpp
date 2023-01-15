#include "Chat.h"
#include "Player.h"
#include "ScriptedGossip.h"

#include "mod_accountbound.h"

bool AccountBound::OnGossipHello(Player* player, Creature* creature)
{
    if (!EnableAccountHeirlooms)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("This feature is currently disabled.");
        CloseGossipMenuFor(player);
        return true;
    }

    QueryResult result = LoginDatabase.Query("SELECT `item_id` FROM `account_bound_heirlooms` WHERE `account_id` = {}",
            player->GetSession()->GetAccountId());

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
