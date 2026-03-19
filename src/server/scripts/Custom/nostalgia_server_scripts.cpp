#include "ChannelMgr.h"
#include "Channel.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"

// Auto-joins players to the "Nostalgia" custom channel on login.
class AutoJoinChannelPlayerScript : public PlayerScript
{
public:
    AutoJoinChannelPlayerScript() : PlayerScript("AutoJoinChannelPlayerScript") {}

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        ChannelMgr* mgr = ChannelMgr::forTeam(player->GetTeamId());
        if (!mgr)
            return;

        Channel* channel = mgr->GetJoinChannel("Nostalgia", 0);
        if (!channel)
            return;

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

// Cosmetic glyph overrides for Moonkin Form.
// When a druid in Moonkin Form has one of these glyph auras, the display
// model is replaced. The hook fires from within SetDisplayId(), so we
// avoid recursion by only acting when the incoming displayId is a stock
// moonkin model — the replacement ID (native model, harpy, etc.) will
// never match, so the re-entry exits immediately.
enum MoonkinGlyphSpells
{
    SPELL_GLYPH_OF_THE_UNFEATHERED = 200137, // keep player's normal appearance
    SPELL_GLYPH_OF_THE_HARPY       = 200139  // harpy creature model
};

enum MoonkinDisplayIds
{
    DISPLAY_MOONKIN_ALLIANCE = 15374, // SpellShapeshiftForm.dbc modelID_A
    DISPLAY_MOONKIN_HORDE    = 15375, // SpellShapeshiftForm.dbc modelID_H
    DISPLAY_NORTHSPRING_HARPY = 10872
};

class MoonkinGlyphUnitScript : public UnitScript
{
public:
    MoonkinGlyphUnitScript() : UnitScript("MoonkinGlyphUnitScript") {}

    void OnDisplayIdChange(Unit* unit, uint32 displayId) override
    {
        if (displayId != DISPLAY_MOONKIN_ALLIANCE && displayId != DISPLAY_MOONKIN_HORDE)
            return;

        Player* player = unit->ToPlayer();
        if (!player)
            return;

        if (player->HasAura(SPELL_GLYPH_OF_THE_UNFEATHERED))
        {
            player->SetDisplayId(player->GetNativeDisplayId());
            return;
        }

        if (player->HasAura(SPELL_GLYPH_OF_THE_HARPY))
        {
            player->SetDisplayId(DISPLAY_NORTHSPRING_HARPY);
            return;
        }
    }
};

void AddSC_nostalgia_server_scripts()
{
    new AutoJoinChannelPlayerScript();
    new GatheringXPPlayerScript();
    new MoonkinGlyphUnitScript();
}
