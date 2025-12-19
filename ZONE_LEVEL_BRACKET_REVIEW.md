# Zone Level Bracket Review - Playerbot vs Relocate Script

**Date**: December 18, 2024  
**Server**: testing-azerothcore.rollet.family  
**Purpose**: Ensure consistency between playerbot teleportation rules, mod_player_bot_level_brackets, and the relocate_bots_dynamic.sh script

---

## Executive Summary

### ✅ FINDINGS

1. **PARTIAL ALIGNMENT**: The relocate script uses simplified level brackets that are DIFFERENT from the detailed zone-based brackets used by playerbots
2. **TELEPORTATION RULES**: Playerbots use zone-specific level brackets defined in `PrepareZone2LevelBracket()` and configurable via `playerbots.conf`
3. **LEVEL DISTRIBUTION**: `mod_player_bot_level_brackets` controls the distribution percentage of bots per level range, NOT their zone placement
4. **KEY DISCREPANCY**: The relocate script groups zones broadly (e.g., "levels 1-12"), while playerbots use precise zone brackets (e.g., zone 17 = 10-25)

---

## 1. Playerbot Zone Level Brackets

### Source Location

- **File**: `testing-azerothcore-wotlk/modules/mod-playerbots/src/RandomPlayerbotMgr.cpp`
- **Function**: `PrepareZone2LevelBracket()`
- **Configuration**: `testing-azerothcore-wotlk/env/dist/etc/modules/playerbots.conf`

### Zone Bracket Definitions (Playerbots)

#### Starter Zones (Levels 5-12)

```cpp
zone2LevelBracket[1] = {5, 12};     // Dun Morogh
zone2LevelBracket[12] = {5, 12};    // Elwynn Forest
zone2LevelBracket[14] = {5, 12};    // Durotar
zone2LevelBracket[85] = {5, 12};    // Tirisfal Glades
zone2LevelBracket[141] = {5, 12};   // Teldrassil
zone2LevelBracket[215] = {5, 12};   // Mulgore
zone2LevelBracket[3430] = {5, 12};  // Eversong Woods
zone2LevelBracket[3524] = {5, 12};  // Azuremyst Isle
```

#### Mid-Level Zones (Levels 10-25)

```cpp
zone2LevelBracket[17] = {10, 25};    // Barrens
zone2LevelBracket[38] = {10, 20};    // Loch Modan
zone2LevelBracket[40] = {10, 21};    // Westfall
zone2LevelBracket[130] = {10, 23};   // Silverpine Forest
zone2LevelBracket[148] = {10, 21};   // Darkshore
zone2LevelBracket[3433] = {10, 22};  // Ghostlands
zone2LevelBracket[3525] = {10, 21};  // Bloodmyst Isle
```

#### Higher Mid-Level Zones (Levels 18-42)

```cpp
zone2LevelBracket[10] = {19, 33};   // Deadwind Pass (actually Duskwood)
zone2LevelBracket[11] = {21, 30};   // Wetlands
zone2LevelBracket[44] = {16, 28};   // Redridge Mountains
zone2LevelBracket[267] = {20, 34};  // Hillsbrad Foothills
zone2LevelBracket[331] = {18, 33};  // Ashenvale
zone2LevelBracket[400] = {24, 36};  // Thousand Needles
zone2LevelBracket[406] = {16, 29};  // Stonetalon Mountains
zone2LevelBracket[45] = {30, 42};   // Arathi Highlands
zone2LevelBracket[405] = {30, 41};  // Desolace
```

#### High-Level Classic Zones (Levels 32-52)

```cpp
zone2LevelBracket[33] = {32, 47};   // Stranglethorn Vale
zone2LevelBracket[440] = {41, 52};  // Tanaris (actually {42, 51} in comments)
zone2LevelBracket[357] = {40, 52};  // Feralas
zone2LevelBracket[3] = {36, 46};    // Badlands
zone2LevelBracket[8] = {36, 46};    // Swamp of Sorrows
zone2LevelBracket[15] = {35, 46};   // Dustwallow Marsh
zone2LevelBracket[16] = {45, 52};   // Azshara
zone2LevelBracket[47] = {42, 51};   // Hinterlands
zone2LevelBracket[51] = {45, 51};   // Searing Gorge
```

#### End-Game Classic Zones (Levels 47-63)

```cpp
zone2LevelBracket[490] = {49, 56};   // Un'Goro Crater
zone2LevelBracket[618] = {54, 61};   // Winterspring
zone2LevelBracket[4] = {52, 57};     // Blasted Lands
zone2LevelBracket[28] = {50, 60};    // Western Plaguelands
zone2LevelBracket[46] = {51, 60};    // Burning Steppes
zone2LevelBracket[139] = {54, 62};   // Eastern Plaguelands
zone2LevelBracket[361] = {47, 57};   // Felwood
zone2LevelBracket[1377] = {54, 63};  // Silithus
```

#### TBC Zones (Levels 58-73)

```cpp
zone2LevelBracket[3483] = {58, 66};  // Hellfire Peninsula
zone2LevelBracket[3521] = {60, 67};  // Zangarmarsh
zone2LevelBracket[3519] = {62, 73};  // Terokkar Forest
zone2LevelBracket[3518] = {64, 70};  // Nagrand
zone2LevelBracket[3522] = {64, 73};  // Blade's Edge Mountains
zone2LevelBracket[3520] = {66, 73};  // Shadowmoon Valley
zone2LevelBracket[3523] = {67, 73};  // Netherstorm
zone2LevelBracket[4080] = {68, 73};  // Isle of Quel'Danas
```

#### WotLK Zones (Levels 68-80)

```cpp
zone2LevelBracket[3537] = {68, 75};  // Borean Tundra
zone2LevelBracket[495] = {68, 74};   // Howling Fjord
zone2LevelBracket[65] = {71, 77};    // Dragonblight
zone2LevelBracket[394] = {72, 78};   // Grizzly Hills
zone2LevelBracket[3711] = {75, 80};  // Sholazar Basin
zone2LevelBracket[66] = {74, 80};    // Zul'Drak
zone2LevelBracket[67] = {77, 80};    // Storm Peaks
zone2LevelBracket[210] = {77, 80};   // Icecrown Glacier
zone2LevelBracket[2817] = {77, 80};  // Crystalsong Forest
zone2LevelBracket[4197] = {79, 80};  // Wintergrasp
```

---

## 2. Relocate Script Zone Assignments

### Source Location

- **File**: `testing-azerothcore-wotlk/relocate_bots_dynamic.sh`
- **Script**: V10 - Using playerbots_random_bots table

### Level Bracket Assignments (Relocate Script)

#### Levels 1-12: STARTER ZONES

```sql
-- Alliance: Race-specific starter zones
WHEN race = 1 THEN zone = 12     -- Human -> Elwynn Forest
WHEN race = 3 THEN zone = 1      -- Dwarf -> Dun Morogh
WHEN race = 4 THEN zone = 141    -- Night Elf -> Teldrassil
WHEN race = 7 THEN zone = 1      -- Gnome -> Dun Morogh
WHEN race = 11 THEN zone = 3524  -- Draenei -> Azuremyst Isle

-- Horde: Race-specific starter zones
WHEN race = 2 THEN zone = 14     -- Orc -> Durotar
WHEN race = 5 THEN zone = 85     -- Undead -> Tirisfal Glades
WHEN race = 6 THEN zone = 215    -- Tauren -> Mulgore
WHEN race = 8 THEN zone = 14     -- Troll -> Durotar
WHEN race = 10 THEN zone = 3430  -- Blood Elf -> Eversong Woods
```

#### Levels 13-20: SECONDARY ZONES

```sql
-- Alliance: Westfall (zone 40) - playerbot bracket is 10-21
WHERE level BETWEEN 13 AND 20 AND race IN (1,3,4,7,11)

-- Horde: Barrens (zone 17) - playerbot bracket is 10-25
WHERE level BETWEEN 13 AND 20 AND race IN (2,5,6,8,10)
```

**❌ MISMATCH**: Relocate uses 13-20, but playerbot brackets show:

- Westfall (zone 40): 10-21
- Barrens (zone 17): 10-25

#### Levels 21-30: MID-LEVEL ZONES

```sql
-- Alliance: Duskwood (zone 10) - playerbot bracket is 19-33
WHERE level BETWEEN 21 AND 30

-- Horde: Ashenvale (zone 331) - playerbot bracket is 18-33
WHERE level BETWEEN 21 AND 30
```

**✅ REASONABLE**: Fits within playerbot brackets, but narrower range

#### Levels 31-40: HIGHER MID-LEVEL ZONES

```sql
-- Alliance: Arathi Highlands (zone 45) - playerbot bracket is 30-42
WHERE level BETWEEN 31 AND 40

-- Horde: Stranglethorn Vale (zone 33) - playerbot bracket is 32-47
WHERE level BETWEEN 31 AND 40
```

**✅ REASONABLE**: Fits within playerbot brackets

#### Levels 41-50: HIGH-LEVEL CLASSIC

```sql
-- Both factions: Tanaris (zone 47) - playerbot bracket is 42-51
WHERE level BETWEEN 41 AND 50
```

**❌ MINOR ISSUE**: Comment says zone 47 (Hinterlands), but means zone 440 (Tanaris)
**✅ RANGE OK**: Fits within playerbot bracket 42-51

#### Levels 51-60: END-GAME CLASSIC

```sql
-- 51-55: Un'Goro Crater (zone 490) - playerbot bracket is 49-56
-- 56-60: Winterspring (zone 618) - playerbot bracket is 54-61
```

**✅ REASONABLE**: Fits within playerbot brackets

#### Levels 61-70: TBC ZONES

```sql
-- Both factions: Shattrath City (zone 3703)
WHERE level BETWEEN 61 AND 70
```

**⚠️ NOTE**: Shattrath (3703) is NOT in the playerbot zone2LevelBracket map!
This means playerbots won't naturally teleport here via `PrepareZone2LevelBracket()`

#### Levels 71-80: WOTLK ZONES

```sql
-- Both factions: Dalaran (zone 4395)
WHERE level BETWEEN 71 AND 80
```

**⚠️ NOTE**: Dalaran (4395) is NOT in the playerbot zone2LevelBracket map!
This means playerbots won't naturally teleport here via `PrepareZone2LevelBracket()`

---

## 3. mod_player_bot_level_brackets Configuration

### Source Location

- **File**: `testing-azerothcore-wotlk/env/dist/etc/modules/mod_player_bot_level_brackets.conf`
- **Module**: `mod-player-bot-level-brackets`

### Current Configuration

```conf
BotLevelBrackets.NumRanges = 11
BotLevelBrackets.Dynamic.SyncFactions = 1

# Alliance Level Brackets (Both factions identical due to SyncFactions=1)
Range1:  Levels 1-9    = 5%
Range2:  Levels 10-19  = 6%
Range3:  Levels 20-29  = 10%
Range4:  Levels 30-39  = 9%
Range5:  Levels 40-49  = 8%
Range6:  Levels 50-59  = 6%
Range7:  Level 60      = 12%
Range8:  Levels 61-69  = 7%
Range9:  Level 70      = 10%
Range10: Levels 71-79  = 16%
Range11: Level 80      = 11%
```

### Important Notes

1. **PURPOSE**: This module controls the **distribution percentage** of bot levels, NOT their zone placement
2. **LEVEL CHANGES**: When bots need to be re-leveled to match desired percentages, they are changed but NOT automatically relocated
3. **ZONE PLACEMENT**: This is SEPARATE from teleportation logic - the module does NOT handle zone assignments

---

## 4. Playerbot Teleportation Logic

### How Playerbots Select Zones

#### Function: `RandomTeleportForLevel(Player* bot)`

**Location**: `RandomPlayerbotMgr.cpp:4729`

```cpp
void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot)
{
    uint32 level = bot->GetLevel();
    uint8 race = bot->getRace();
    std::vector<WorldLocation>* locs = nullptr;

    if (sPlayerbotAIConfig->enableNewRpgStrategy)
        locs = IsAlliance(race) ? &allianceStarterPerLevelCache[level]
                                : &hordeStarterPerLevelCache[level];
    else
        locs = &locsPerLevelCache[level];

    RandomTeleport(bot, *locs);
}
```

#### Cache Building Process

1. **PrepareZone2LevelBracket()** - Defines zone level ranges
2. **PrepareTeleportCache()** - Queries innkeepers/flightmasters from database
3. For each innkeeper/flightmaster:
    - Get zone ID from map coordinates
    - Look up zone bracket from `zone2LevelBracket` map
    - Add location to cache for ALL levels in that bracket
    - Faction-specific: Check if location is hostile to faction

#### Key Code Section

```cpp
const AreaTableEntry* area = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
uint32 zoneId = area->zone ? area->zone : area->ID;

if (zone2LevelBracket.find(zoneId) == zone2LevelBracket.end())
    continue;  // Skip zones not in bracket map

LevelBracket bracket = zone2LevelBracket[zoneId];
for (int i = bracket.low; i <= bracket.high; i++)
{
    if (forHorde)
        hordeStarterPerLevelCache[i].push_back(loc);
    if (forAlliance)
        allianceStarterPerLevelCache[i].push_back(loc);
}
```

### Teleportation Safety Checks

From `RandomTeleport()` function:

```cpp
// Do not teleport to enemy zones if level is low
if (zone->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
    continue;

if (zone->team == 2 && bot->GetTeamId() == TEAM_HORDE)
    continue;
```

---

## 5. Critical Discrepancies

### Issue #1: Overlapping Level Ranges

**Problem**: Relocate script uses non-overlapping ranges (1-12, 13-20, 21-30), but playerbots use overlapping brackets

**Example**:

- **Relocate**: Levels 13-20 → Westfall (zone 40)
- **Playerbot**: Zone 40 (Westfall) = levels 10-21

**Impact**:

- Level 10-12 bots relocated to starter zones might be teleported to Westfall by playerbots
- Level 13-20 bots in Westfall might be teleported back to starter zones if they're also valid for starter zones

### Issue #2: Missing City Zones

**Problem**: Shattrath (3703) and Dalaran (4395) are NOT in `zone2LevelBracket` map

**Impact**:

- Bots relocated to these cities won't naturally roam there via playerbots
- They'll only go there if explicitly teleported or for banker visits
- Playerbots may try to teleport them to leveling zones instead

### Issue #3: Incorrect Zone Reference

**Problem**: Comment in relocate script says "Tanaris (zone 47)" but 47 is Hinterlands

**Correction Needed**:

```sql
-- Should be: Tanaris (zone 440, bracket 42-51)
-- Currently says: zone 47 (which is actually Hinterlands, bracket 42-51)
```

### Issue #4: Hinterlands vs Tanaris Confusion

**Relocate Script**: Uses zone 47 in comment
**Actual Database**: Zone 47 = Hinterlands (42-51), Zone 440 = Tanaris (42-51)

Both zones have the same bracket, so functionally it works, but documentation is misleading

---

## 6. Recommendations

### Priority 1: Align Level Ranges ⚠️ HIGH

**Action**: Update `relocate_bots_dynamic.sh` to match playerbot zone brackets

```bash
# CURRENT (Levels 13-20 to Westfall)
WHERE level BETWEEN 13 AND 20

# RECOMMENDED (Match playerbot bracket 10-21)
WHERE level BETWEEN 10 AND 21
```

**Zones to Update**:

- Westfall (40): Change from 13-20 to 10-21
- Barrens (17): Change from 13-20 to 10-25
- Duskwood (10): Change from 21-30 to 19-33
- Ashenvale (331): Change from 21-30 to 18-33
- Stranglethorn Vale (33): Already 31-40, but bracket is 32-47 (acceptable)
- Arathi Highlands (45): Already 31-40, but bracket is 30-42 (acceptable)

### Priority 2: Add City Zones to Playerbot Brackets 📝 MEDIUM

**Action**: Add Shattrath and Dalaran to `PrepareZone2LevelBracket()`

```cpp
// Add to RandomPlayerbotMgr.cpp PrepareZone2LevelBracket()
zone2LevelBracket[3703] = {60, 70};  // Shattrath City (TBC hub)
zone2LevelBracket[4395] = {70, 80};  // Dalaran (WotLK hub)
```

**Alternative**: Keep cities out of brackets intentionally (they're banker destinations, not leveling zones)

### Priority 3: Fix Documentation 📝 LOW

**Action**: Correct zone ID comments in relocate script

```sql
-- BEFORE
-- Tanaris (zone 47, bracket 42-51)

-- AFTER
-- Tanaris (zone 440, bracket 42-51)
-- Note: Zone 47 is Hinterlands (also 42-51)
```

### Priority 4: Consider Starter Zone Overlap ⚠️ MEDIUM

**Problem**: Levels 10-12 are in BOTH starter zones (5-12) and secondary zones (10-21 Westfall, 10-25 Barrens)

**Options**:

1. **Keep overlap** - Bots can naturally progress from starter to secondary zones
2. **Remove overlap** - Change starter zones to 5-9, secondary to 10-21
3. **Relocate only to starters** - Keep levels 10-12 in starter zones during relocation

**Recommendation**: Keep the overlap - it's natural for level 10-12 bots to roam between starter and secondary zones

---

## 7. Proposed Updated Relocate Script

Here's a revised SQL that aligns better with playerbot brackets:

```sql
-- ============================================================================
-- LEVELS 1-9: STARTER ZONES (matches mod_player_bot_level_brackets Range1)
-- ============================================================================
WHERE level BETWEEN 1 AND 9

-- ============================================================================
-- LEVELS 10-21: SECONDARY ZONES (overlaps with starter zones 10-12)
-- ============================================================================
-- Alliance: Westfall (zone 40, playerbot bracket 10-21)
WHERE level BETWEEN 10 AND 21 AND race IN (1,3,4,7,11)

-- Horde: Barrens (zone 17, playerbot bracket 10-25)
WHERE level BETWEEN 10 AND 25 AND race IN (2,5,6,8,10)

-- ============================================================================
-- LEVELS 19-33: MID-LEVEL ZONES
-- ============================================================================
-- Alliance: Duskwood (zone 10, playerbot bracket 19-33)
WHERE level BETWEEN 19 AND 33 AND race IN (1,3,4,7,11)

-- Horde: Ashenvale (zone 331, playerbot bracket 18-33)
WHERE level BETWEEN 18 AND 33 AND race IN (2,5,6,8,10)

-- ============================================================================
-- LEVELS 30-42: HIGHER MID-LEVEL ZONES
-- ============================================================================
-- Alliance: Arathi Highlands (zone 45, playerbot bracket 30-42)
WHERE level BETWEEN 30 AND 42 AND race IN (1,3,4,7,11)

-- Horde: Stranglethorn Vale (zone 33, playerbot bracket 32-47)
WHERE level BETWEEN 32 AND 47 AND race IN (2,5,6,8,10)

-- ============================================================================
-- LEVELS 42-51: HIGH-LEVEL CLASSIC ZONES
-- ============================================================================
-- Both factions: Tanaris (zone 440, playerbot bracket 42-51)
WHERE level BETWEEN 42 AND 51

-- ============================================================================
-- LEVELS 49-56: END-GAME CLASSIC (UN'GORO)
-- ============================================================================
-- Both factions: Un'Goro Crater (zone 490, playerbot bracket 49-56)
WHERE level BETWEEN 49 AND 56

-- ============================================================================
-- LEVELS 54-61: END-GAME CLASSIC (WINTERSPRING)
-- ============================================================================
-- Both factions: Winterspring (zone 618, playerbot bracket 54-61)
WHERE level BETWEEN 54 AND 61

-- ============================================================================
-- LEVELS 58-70: BURNING CRUSADE ZONES
-- ============================================================================
-- Option 1: Use Shattrath City (zone 3703) - hub city, safe for all
-- Option 2: Use TBC leveling zones with specific brackets:
--   - Hellfire Peninsula (3483): 58-66
--   - Zangarmarsh (3521): 60-67
--   - Terokkar Forest (3519): 62-73
--   - Nagrand (3518): 64-70

-- ============================================================================
-- LEVELS 68-80: WRATH OF THE LICH KING ZONES
-- ============================================================================
-- Option 1: Use Dalaran (zone 4395) - hub city, safe for all
-- Option 2: Use WotLK leveling zones with specific brackets:
--   - Borean Tundra (3537): 68-75
--   - Howling Fjord (495): 68-74
--   - Dragonblight (65): 71-77
--   - Storm Peaks (67): 77-80
```

---

## 8. Testing Recommendations

### Test Case 1: Starter Zone Transition

**Setup**:

- Create level 10 Alliance bot in Elwynn Forest
- Run relocate script
- Wait for playerbot teleportation

**Expected Result**: Bot should be able to roam between Elwynn Forest and Westfall

### Test Case 2: Hub City Placement

**Setup**:

- Create level 65 bot in Shattrath
- Check if playerbots will teleport them away

**Expected Result**: Bot should stay in TBC zones, possibly being teleported to leveling zones

### Test Case 3: Level Bracket Enforcement

**Setup**:

- Manually place level 20 bot in Winterspring (54-61 bracket)
- Observe playerbot behavior

**Expected Result**: Playerbots should not teleport to Winterspring (outside bracket)

---

## 9. Conclusion

### Summary of Issues

1. ✅ **Zone brackets are defined** - Playerbots have comprehensive zone level brackets
2. ❌ **Relocate script uses simplified ranges** - Different from playerbot brackets
3. ⚠️ **Hub cities not in bracket map** - Shattrath and Dalaran may cause issues
4. ℹ️ **mod_player_bot_level_brackets** - Only controls level distribution, not zone placement

### Key Takeaway

The relocate script should be updated to align with the playerbot zone brackets to ensure consistent bot behavior. The current misalignment means:

- Bots may be relocated to zones outside their level bracket
- Playerbots may immediately teleport bots away from relocated positions
- Level ranges don't match natural zone progression

### Recommended Next Steps

1. Update `relocate_bots_dynamic.sh` to use playerbot zone brackets
2. Add Shattrath (3703) and Dalaran (4395) to `PrepareZone2LevelBracket()` OR keep them as banker-only destinations
3. Test bot behavior after relocation to ensure they stay in appropriate zones
4. Consider adding zone bracket validation to the relocate script

---

## Appendix: Zone ID Reference

| Zone ID | Zone Name          | Playerbot Bracket | Relocate Script Assignment |
| ------- | ------------------ | ----------------- | -------------------------- |
| 1       | Dun Morogh         | 5-12              | 1-12 (Starter)             |
| 12      | Elwynn Forest      | 5-12              | 1-12 (Starter)             |
| 14      | Durotar            | 5-12              | 1-12 (Starter)             |
| 17      | Barrens            | 10-25             | 13-20 (Secondary) ❌       |
| 40      | Westfall           | 10-21             | 13-20 (Secondary) ❌       |
| 10      | Duskwood           | 19-33             | 21-30 (Mid) ⚠️             |
| 331     | Ashenvale          | 18-33             | 21-30 (Mid) ⚠️             |
| 45      | Arathi Highlands   | 30-42             | 31-40 (Higher Mid) ✅      |
| 33      | Stranglethorn Vale | 32-47             | 31-40 (Higher Mid) ⚠️      |
| 440     | Tanaris            | 42-51             | 41-50 (High) ⚠️            |
| 490     | Un'Goro Crater     | 49-56             | 51-55 ⚠️                   |
| 618     | Winterspring       | 54-61             | 56-60 ⚠️                   |
| 3703    | Shattrath City     | NOT IN MAP        | 61-70 (TBC) ⚠️             |
| 4395    | Dalaran            | NOT IN MAP        | 71-80 (WotLK) ⚠️           |

**Legend**:

- ✅ Good alignment
- ⚠️ Minor mismatch but functional
- ❌ Significant mismatch requiring attention
