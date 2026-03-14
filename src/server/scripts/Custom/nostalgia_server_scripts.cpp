#include "ChannelMgr.h"
#include "Channel.h"
#include "Player.h"
#include "ScriptMgr.h"

// Auto-joins players to the "Nostalgia" custom channel on login.
class AutoJoinChannelPlayerScript : public PlayerScript
{
public:
    AutoJoinChannelPlayerScript() : PlayerScript("AutoJoinChannelPlayerScript") {}

    void OnLogin(Player* player) override
    {
        if (!player)
            return;

        ChannelMgr* mgr = ChannelMgr::forTeam(player->GetTeamId());
        if (!mgr)
            return;

        Channel* channel = mgr->GetJoinChannel("Nostalgia", 0);
        if (!channel)
            return;

        if (!channel->IsOn(player->GetGUID()))
            channel->JoinChannel(player, "");
    }
};

// Awards XP on every successful gather (Herbalism, Mining, Skinning).
// Linear decay from ~2.5% of a level at level 1 to ~0.5% at level 80
// (~1% at level 60).
class GatheringXPPlayerScript : public PlayerScript
{
public:
    GatheringXPPlayerScript() : PlayerScript("GatheringXPPlayerScript") {}

    void OnPlayerUpdateGatheringSkill(Player* player, uint32 skillId,
        uint32 /*current*/, uint32 /*gray*/, uint32 /*green*/,
        uint32 /*yellow*/, uint32& /*gain*/) override
    {
        if (skillId != SKILL_HERBALISM &&
            skillId != SKILL_MINING   &&
            skillId != SKILL_SKINNING)
            return;

        // Linear decay: 2.5% at lvl 1, ~1% at lvl 60, 0.5% at lvl 80
        float fraction = 0.005f + 0.02f * (80.0f - float(player->GetLevel())) / 79.0f;
        uint32 xp = uint32(player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP) * fraction);
        player->GiveXP(xp, nullptr);
    }
};

void AddSC_nostalgia_server_scripts()
{
    new AutoJoinChannelPlayerScript();
    new GatheringXPPlayerScript();
}
