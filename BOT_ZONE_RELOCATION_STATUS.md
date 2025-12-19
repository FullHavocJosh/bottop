# Bot Zone Relocation & ZoneBracket Enforcement - Status Report

**Date**: December 17, 2025  
**Server**: `ssh root@testing-azerothcore.rollet.family`  
**Project Directory**: `/root/testing-azerothcore-wotlk`

---

## Executive Summary

**Goal**: Ensure playerbots spawn and remain in zones appropriate for their level based on ZoneBracket definitions in `playerbots.conf`.

**Current Status**: ⚠️ **PARTIALLY WORKING BUT BROKEN**

- Bot relocation script (V8) runs at startup and places bots in zones
- Runtime teleport validation code exists but cannot prevent issues caused by bad initial placement
- **Core Issue**: Relocation script has incorrect coordinates/logic, causing bots to spawn in wrong zones despite SQL saying otherwise

**Key Insight**: Setting `zone = X` in SQL doesn't guarantee the bot spawns in zone X. The bot spawns at coordinates (x,y,z) and the server calculates the actual zone from those coordinates.

---

## Problem Background

### Original Issue

After server restart, high-level bots (60-80) were appearing in low-level starter zones (1-12), and vice versa. This violates ZoneBracket definitions in `playerbots.conf`.

### Root Causes Discovered

1. **Overlapping ZoneBrackets in V7 Script**
    - Multiple zones claimed the same level ranges
    - Example: Zone 12 (1-12) AND Zone 40 (10-21) both UPDATE level 10-12 bots
    - Last UPDATE statement "wins", causing unpredictable placement

2. **Mismatch Between Script and Config**
    - V8 script initially used 1-9 for starter zones
    - `playerbots.conf` defines starter zones as 1-12
    - Validation allowed level 10-12 bots in starter zones (technically valid per config)

3. **Coordinates Don't Match Zone IDs**
    - Setting `zone = 33` in SQL doesn't teleport bot to zone 33
    - Bot spawns at (x, y, z) coordinates, server calculates actual zone
    - **Critical**: If coordinates are wrong, bot ends up in different zone than SQL specifies

4. **sed Commands Broke V8 Script**
    - Attempted to fix level ranges with global sed replacements
    - Updated ALL occurrences, breaking both Alliance and Horde sections
    - Comments no longer match SQL WHERE clauses
    - Example: Comment says "Levels 30-35" but SQL says "WHERE level BETWEEN 36 AND 45"

---

## What Has Been Done

### 1. Created V7 Bot Relocation Script ✅

**File**: `/root/testing-azerothcore-wotlk/relocate_bots_dynamic.sh`

**Purpose**: Relocate all 16,500 bots to level-appropriate zones BEFORE worldserver starts

**Approach**:

- Reads ZoneBracket definitions from `playerbots.conf`
- Generates SQL with multiple UPDATE statements
- Each UPDATE targets a specific level range and zone
- Runs via docker-compose dependency before worldserver

**Problem**: Had overlapping level ranges causing bots to be moved multiple times

---

### 2. Created V8 Bot Relocation Script ✅

**File**: Same as V7, replaced in place

**Improvements**:

- **Non-overlapping level ranges**: Each level maps to exactly ONE zone
- Uses `CASE` statements based on race for starter zones
- Faction filtering (Alliance vs Horde vs Neutral zones)

**Original Level Ranges** (V8 initial):

```
1-9:   Starter zones (by race)
10-19: Secondary zones
20-29: Mid-level zones
30-35: Higher-level zones
36-41: High-level zones
42-49: Classic endgame
50-60: Classic max
61-70: Burning Crusade
71-80: Wrath of the Lich King
```

**Modified Level Ranges** (V8 after sed):

```
1-12:  Starter zones (by race)
13-20: Secondary zones
21-35: Mid-level zones
36-45: Higher-level zones
46-52: High-level zones
53-60: Classic max (DUPLICATE - see issue below)
61-70: Burning Crusade
71-80: Wrath of the Lich King
```

**Current Status**: ⚠️ **BROKEN** - sed commands updated level ranges incorrectly, causing mismatches

---

### 3. Added ZoneBracket Validation to RandomTeleport() ✅

**File**: `/root/testing-azerothcore-wotlk/modules/mod-playerbots/src/RandomPlayerbotMgr.cpp`  
**Backup**: `/root/testing-azerothcore-wotlk/modules/mod-playerbots/src/RandomPlayerbotMgr.cpp.backup`

**Code Added** (after line 1730):

```cpp
LOG_INFO("playerbots", "Bot {} (level {}) attempting teleport to zone {}",
         bot->GetName().c_str(), bot->GetLevel(), zone->ID);

// ADDED: Validate bot level matches zone's ZoneBracket (NO EXCEPTIONS)
uint32 botLevel = bot->GetLevel();

auto bracketIt = zone2LevelBracket.find(zone->ID);
if (bracketIt != zone2LevelBracket.end())
{
    uint32 minLevel = bracketIt->second.low;
    uint32 maxLevel = bracketIt->second.high;

    if (botLevel < minLevel || botLevel > maxLevel)
    {
        LOG_INFO("playerbots", "Bot {} (level {}) BLOCKED from zone {} (bracket {}-{})",
                 bot->GetName().c_str(), botLevel, zone->ID, minLevel, maxLevel);
        continue;  // Skip this location, try next one
    }
}
// END ADDED
```

**How It Works**:

- Intercepts EVERY teleport attempt in `RandomTeleport()`
- Checks if destination zone has a ZoneBracket definition
- If yes, validates bot level is within bracket range
- If invalid, skips that location and tries next teleport candidate

**Current Observations**:

- Logs show "attempting teleport" messages ✅
- **Zero "BLOCKED" messages** ❌
- This means either:
    - All teleports are actually valid (unlikely given data)
    - Zones don't have ZoneBracket definitions (validation skipped)
    - Initial placement is wrong, bots never try to leave

**Note**: `zone2LevelBracket` map is populated by `PrepareZone2LevelBracket()` function, which:

1. Hardcodes default values
2. Reads from `sPlayerbotAIConfig->zoneBrackets` (loaded from playerbots.conf)
3. Config values override hardcoded defaults

---

### 4. Tested and Monitored Results ⚠️

**Immediate After Relocation (V8 with 1-12 ranges)**:

```
Zone 14 (Durotar):     296 bots, levels 1-12, 0 above level 12 ✅
Zone 1 (Dun Morogh):   256 bots, levels 1-12, 0 above level 12 ✅
Zone 3430 (Eversong):  253 bots, levels 1-12, 0 above level 12 ✅
Zone 3524 (Azuremyst): 248 bots, levels 1-12, 0 above level 12 ✅
```

**1 Minute After Worldserver Start**:

```
Zone 14 (Durotar):     417 bots, levels 1-45, 121 above level 12 ❌
Zone 1 (Dun Morogh):   256 bots, levels 1-30, 1 above level 12 ⚠️
Zone 3430 (Eversong):  253 bots, levels 1-12, 0 above level 12 ✅
Zone 3524 (Azuremyst): 248 bots, levels 1-12, 0 above level 12 ✅
```

**Analysis**:

- Zone 14 gained 121 bots (296 → 417)
- High-level bots are levels 36-45
- These bots were relocated there by V8 script (wrong coordinates/logic)
- NOT caused by runtime teleportation (validation would show BLOCKED logs)

---

## Current Issues

### Issue #1: V8 Script Has Wrong Coordinates ❌

**Symptom**: Bots placed in zone X end up in zone Y

**Example**:

```sql
-- Comment says "Levels 30-35: Arathi Highlands"
UPDATE characters
SET position_x = 1357.10, position_y = -4412.01, position_z = 28.38,
    map = 1, zone = 33, orientation = 0  -- Sets zone=33 in SQL
WHERE level BETWEEN 36 AND 45            -- But level range is wrong too!
  AND race IN (2, 5, 6, 8, 10)
```

**Problem**:

- SQL sets `zone = 33` (Stranglethorn Vale)
- BUT coordinates might actually be in zone 14 (Durotar)
- Server calculates zone from (x,y,z), ignoring SQL's zone value
- Bot ends up in wrong zone

---

### Issue #2: Comments Don't Match SQL ❌

**Symptom**: Human-readable comments contradict SQL logic

**Example**:

```sql
-- Levels 30-35: Badlands          <-- Comment says 30-35
WHERE level BETWEEN 36 AND 45      <-- SQL says 36-45
```

**Cause**: Global sed replacements updated level ranges everywhere, including wrong places

---

### Issue #3: Duplicate Level Ranges ❌

**Symptom**: Same level range used multiple times

**Example**:

```sql
-- Levels 42-49: Tanaris
WHERE level BETWEEN 53 AND 60

-- Levels 50-60: Burning Steppes
WHERE level BETWEEN 53 AND 60      <-- Same range!
```

**Result**: Second UPDATE overwrites first, some zones never get bots

---

### Issue #4: Validation Can't Fix Bad Initial Placement ❌

**Symptom**: Zero "BLOCKED" logs despite bots in wrong zones

**Why**:

- Bots are placed in wrong zones by relocation script
- Bots don't try to teleport OUT of their current zone (it's "valid" per their level)
- Validation only runs when bot attempts NEW teleport
- If bot is happy where it is, validation never triggers

**Example**:

- Level 40 bot placed in zone 14 (Durotar, 1-12) by broken script
- Zone 14 is "home" for this bot now
- Bot might never trigger a random teleport, or if it does, it tries to go to valid zones
- Validation sees "level 40 bot wants to go to zone 46 (51-60)" and allows it (even though bot is currently in wrong zone)

---

## What Still Needs to Be Done

### Priority 1: Create Clean V9 Relocation Script ⚠️ **CRITICAL**

**Requirements**:

1. **Non-overlapping level ranges** matching playerbots.conf ZoneBracket definitions
2. **Verified coordinates** for each target zone
3. **Proper zone ID calculation** - coordinates MUST be within the specified zone
4. **Clear structure** - comments match SQL, no ambiguity

**Recommended Level Ranges** (based on playerbots.conf):

```
Starter Zones (1-12):
  - Zone 12:   Elwynn Forest (Alliance)
  - Zone 1:    Dun Morogh (Alliance)
  - Zone 141:  Teldrassil (Alliance)
  - Zone 3524: Azuremyst Isle (Alliance)
  - Zone 14:   Durotar (Horde)
  - Zone 85:   Tirisfal Glades (Horde)
  - Zone 215:  Mulgore (Horde)
  - Zone 3430: Eversong Woods (Horde)

Secondary Zones (13-25):
  - Zone 40:   Westfall (Alliance, 10-21)
  - Zone 38:   Loch Modan (Alliance, 10-20)
  - Zone 148:  Darkshore (Alliance, 10-21)
  - Zone 17:   Barrens (Horde, 10-25)
  - Zone 130:  Silverpine (Horde, 10-23)
  - Zone 3433: Ghostlands (Horde, 10-22)

Mid-Level (26-40):
  - Zone 267:  Hillsbrad (20-34)
  - Zone 331:  Ashenvale (18-33)
  - Zone 405:  Desolace (30-41)

High-Level (41-60):
  - Zone 47:   Tanaris (42-51)
  - Zone 46:   Burning Steppes (51-60)
  - Zone 28:   Western Plaguelands (50-60)

BC (61-70):
  - Zone 3703: Shattrath (61-70)
  - Zone 3483: Hellfire Peninsula (58-66)

WotLK (71-80):
  - Zone 4395: Dalaran (71-80)
  - Zone 3537: Borean Tundra (68-75)
  - Zone 495:  Howling Fjord (68-74)
```

**How to Get Correct Coordinates**:

Option A: Query existing bot positions in correct zones

```sql
SELECT zone, AVG(position_x), AVG(position_y), AVG(position_z), map
FROM characters
WHERE account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%')
  AND zone = 14
GROUP BY zone, map;
```

Option B: Use WoWHead or reference data for zone center coordinates

Option C: Copy coordinates from existing working zones in playerbots codebase

**Script Structure** (suggested):

```bash
#!/bin/bash
# V9 - Clean relocation with verified coordinates

# For each level bracket:
#   1. Define target zones (Alliance, Horde, Neutral)
#   2. Use verified coordinates for each zone
#   3. Single UPDATE per faction+level combination
#   4. No overlapping ranges

# Example:
UPDATE characters SET
    position_x = CASE
        WHEN race IN (1,3,4,7,11) AND MOD(guid, 4) = 0 THEN -8949.95  -- Elwynn
        WHEN race IN (1,3,4,7,11) AND MOD(guid, 4) = 1 THEN -6240.32  -- Dun Morogh
        WHEN race IN (1,3,4,7,11) AND MOD(guid, 4) = 2 THEN 10311.30  -- Teldrassil
        WHEN race IN (1,3,4,7,11) AND MOD(guid, 4) = 3 THEN -3961.67  -- Azuremyst
        WHEN race IN (2,5,6,8,10) AND MOD(guid, 4) = 0 THEN -618.52   -- Durotar
        -- etc...
    END,
    -- position_y, position_z, map, zone similarly
WHERE level BETWEEN 1 AND 12
  AND account IN (...);
```

---

### Priority 2: Verify ZoneBracket Definitions in playerbots.conf ⚠️

**File**: `/root/testing-azerothcore-wotlk/env/dist/etc/modules/playerbots.conf`

**Check**:

1. All zones used in V9 script have ZoneBracket definitions
2. Level ranges are correct and make sense for zone progression
3. No conflicting definitions

**Command**:

```bash
ssh root@testing-azerothcore.rollet.family \
  "grep '^AiPlayerbot.ZoneBracket' /root/testing-azerothcore-wotlk/env/dist/etc/modules/playerbots.conf"
```

**Known Good Examples**:

```
AiPlayerbot.ZoneBracket.1 = 1,12
AiPlayerbot.ZoneBracket.12 = 1,12
AiPlayerbot.ZoneBracket.14 = 1,12
AiPlayerbot.ZoneBracket.17 = 10,25
AiPlayerbot.ZoneBracket.46 = 51,60
AiPlayerbot.ZoneBracket.3703 = 61,70
AiPlayerbot.ZoneBracket.4395 = 71,80
```

---

### Priority 3: Test V9 Script Thoroughly ✅

**Testing Process**:

1. Stop services: `cd /root/testing-azerothcore-wotlk && docker compose down`
2. Start services: `docker compose up -d`
3. Check relocation logs: `docker logs testing-ac-bot-relocate`
4. **Immediately** check zone distribution (before worldserver interference):

```sql
SELECT
    c.zone,
    COUNT(*) as total_bots,
    MIN(c.level) as min_lvl,
    MAX(c.level) as max_lvl,
    COUNT(CASE WHEN c.level > 12 THEN 1 END) as bots_above_12
FROM characters c
WHERE c.account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%')
  AND c.zone IN (12, 1, 141, 3524, 14, 85, 215, 3430)
GROUP BY c.zone;
```

**Success Criteria**:

- Starter zones (1,12,14,85,141,215,3430,3524) have ONLY levels 1-12
- No "bots_above_12" should be > 0 for starter zones
- All 16,500 bots should be relocated (check count)
- No bots in zone 0 or NULL zones

5. Wait for worldserver to load (2-3 minutes)
6. Check zone distribution again
7. Monitor logs for "BLOCKED" messages: `docker logs testing-ac-worldserver | grep BLOCKED`

**If Issues Persist**: The validation code might need adjustment or there may be other teleport functions bypassing it.

---

### Priority 4: Consider Additional Teleport Functions ⚠️

**Current Validation**: Only in `RandomTeleport()` function

**Other Possible Teleport Paths**:

- `RandomTeleportForLevel()` - Wrapper that calls RandomTeleport()
- `RandomTeleportGrindForLevel()` - Might have separate path
- Direct GM commands or special bot behaviors
- Login/logout repositioning

**Action**: Search for other teleport functions:

```bash
ssh root@testing-azerothcore.rollet.family \
  "grep -n 'TeleportTo\|Teleport.*Level\|RandomTeleport' \
   /root/testing-azerothcore-wotlk/modules/mod-playerbots/src/RandomPlayerbotMgr.cpp \
   | head -50"
```

**If Found**: Add same validation to those functions

---

### Priority 5: Enable More Detailed Logging (Optional) 📊

**Current Logging**:

- "Bot X (level Y) attempting teleport to zone Z"
- "Bot X (level Y) BLOCKED from zone Z (bracket A-B)"

**Additional Useful Logs**:

- Current zone bot is teleporting FROM
- Reason for teleport (random, grind, quest, etc.)
- How many candidate locations were checked before finding valid one
- When validation is skipped (zone not in zone2LevelBracket)

**Example Enhanced Logging**:

```cpp
LOG_INFO("playerbots", "Bot {} (level {}) in zone {} attempting teleport to zone {} - {}",
         bot->GetName().c_str(), bot->GetLevel(),
         bot->GetZoneId(), zone->ID,
         bracketIt != zone2LevelBracket.end() ? "checking bracket" : "no bracket, allowing");
```

---

## Key Files Reference

### Scripts

- **Bot Relocation**: `/root/testing-azerothcore-wotlk/relocate_bots_dynamic.sh`
    - Currently V8 (broken)
    - Needs to be replaced with V9

### Configuration

- **ZoneBrackets**: `/root/testing-azerothcore-wotlk/env/dist/etc/modules/playerbots.conf`
    - Defines min/max levels for each zone
    - Example: `AiPlayerbot.ZoneBracket.14 = 1,12`

### Source Code

- **Validation Logic**: `/root/testing-azerothcore-wotlk/modules/mod-playerbots/src/RandomPlayerbotMgr.cpp`
    - Line ~1732: Validation code added
    - Function: `RandomTeleport()`
    - Backup exists: `RandomPlayerbotMgr.cpp.backup`

### Docker

- **Compose File**: `/root/testing-azerothcore-wotlk/docker-compose.yml`
    - Defines service dependencies
    - Ensures bot-relocate runs before worldserver

---

## Useful Commands

### Server Access

```bash
ssh root@testing-azerothcore.rollet.family
cd /root/testing-azerothcore-wotlk
```

### Service Management

```bash
# Stop all services
docker compose down

# Rebuild worldserver (after code changes)
docker compose build ac-worldserver

# Start all services
docker compose up -d

# Check service status
docker compose ps

# View logs
docker logs testing-ac-worldserver -f
docker logs testing-ac-bot-relocate
```

### Database Queries

**Check Starter Zone Distribution**:

```bash
docker exec -i testing-ac-database mysql -uroot -ppassword acore_characters -e '
SELECT
    c.zone,
    COUNT(*) as total_bots,
    MIN(c.level) as min_lvl,
    MAX(c.level) as max_lvl,
    COUNT(CASE WHEN c.level > 12 THEN 1 END) as bots_above_12
FROM characters c
WHERE c.account IN (SELECT id FROM acore_auth.account WHERE username LIKE "RNDBOT%")
  AND c.zone IN (12, 1, 141, 3524, 14, 85, 215, 3430)
GROUP BY c.zone
ORDER BY total_bots DESC;' 2>&1 | grep -v 'Using a password'
```

**Check All Zone Distribution**:

```bash
docker exec -i testing-ac-database mysql -uroot -ppassword acore_characters -e '
SELECT
    c.zone,
    COUNT(*) as total_bots,
    MIN(c.level) as min_lvl,
    MAX(c.level) as max_lvl
FROM characters c
WHERE c.account IN (SELECT id FROM acore_auth.account WHERE username LIKE "RNDBOT%")
GROUP BY c.zone
ORDER BY total_bots DESC
LIMIT 30;' 2>&1 | grep -v 'Using a password'
```

**Check Level Distribution**:

```bash
docker exec -i testing-ac-database mysql -uroot -ppassword acore_characters -e '
SELECT level, COUNT(*) as bot_count
FROM characters
WHERE account IN (SELECT id FROM acore_auth.account WHERE username LIKE "RNDBOT%")
GROUP BY level
ORDER BY level;' 2>&1 | grep -v 'Using a password'
```

**Get Average Coordinates for Zone** (for V9 script):

```bash
docker exec -i testing-ac-database mysql -uroot -ppassword acore_characters -e '
SELECT zone,
       AVG(position_x) as avg_x,
       AVG(position_y) as avg_y,
       AVG(position_z) as avg_z,
       map
FROM characters
WHERE account IN (SELECT id FROM acore_auth.account WHERE username LIKE "RNDBOT%")
  AND zone = 14
GROUP BY zone, map;' 2>&1 | grep -v 'Using a password'
```

### Log Analysis

```bash
# Check for validation attempts
docker logs testing-ac-worldserver 2>&1 | grep "attempting teleport" | head -50

# Check for blocked teleports
docker logs testing-ac-worldserver 2>&1 | grep "BLOCKED" | head -50

# Count blocked teleports
docker logs testing-ac-worldserver 2>&1 | grep "BLOCKED" | wc -l

# Check specific bot's teleports
docker logs testing-ac-worldserver 2>&1 | grep "Bot Griny" | grep teleport
```

---

## Technical Details

### How Zone Calculation Works

When a character is placed at coordinates (x, y, z) on a map:

1. **SQL UPDATE**: Sets position_x, position_y, position_z, map, zone
2. **Server Startup**: Loads character data
3. **Zone Lookup**: Server calls `Map::GetZoneId(phaseMask, x, y, z)`
4. **Zone Override**: Server's calculated zone overrides SQL's zone value
5. **Result**: Character appears in zone calculated from coordinates, NOT from SQL

**Critical Implication**: The `zone` field in SQL is essentially ignored. You MUST use coordinates that are actually within the intended zone.

### How zone2LevelBracket Map Works

1. **Initialization**: `PrepareZone2LevelBracket()` called during server startup
2. **Default Values**: Hardcoded values set for common zones
3. **Config Override**: Reads `sPlayerbotAIConfig->zoneBrackets` from playerbots.conf
4. **Final Map**: Combination of hardcoded + config values

**Structure**:

```cpp
std::map<uint32, LevelBracket> zone2LevelBracket;

struct LevelBracket {
    uint32 low;   // Minimum level
    uint32 high;  // Maximum level
};
```

**Example**:

```cpp
zone2LevelBracket[14] = {1, 12};  // Durotar: levels 1-12
zone2LevelBracket[46] = {51, 60}; // Burning Steppes: levels 51-60
```

### How Validation Works

1. Bot wants to teleport to location with coordinates (x, y, z)
2. Server calculates zone ID from coordinates: `zone = map->GetZoneId(...)`
3. **Validation**: Look up `zone2LevelBracket[zone->ID]`
4. **If found**: Check if bot->GetLevel() is within bracket's low/high range
5. **If valid**: Allow teleport (continue to faction checks, water checks, etc.)
6. **If invalid**: Skip this location (continue to next candidate location)
7. **If not found**: Skip validation, allow teleport

**Key Point**: Validation only happens DURING teleport attempt. If bot is already in wrong zone, validation won't fix it unless bot tries to leave.

---

## Decision Points for Next Session

### Question 1: How to Handle Zone Coordinates?

**Option A**: Use database averages (current bot positions)

- Pro: Guaranteed to be in correct zone (bots are there now)
- Con: Might include wrongly-placed bots, skewing averages
- Con: Might be in weird locations (water, mountains, etc.)

**Option B**: Use hardcoded "safe" coordinates

- Pro: Known good spawn points
- Con: Need to find/verify coordinates for each zone
- Con: Time-consuming to test each one

**Option C**: Query creature spawn locations

- Pro: Game-designed safe locations
- Con: Complex query
- Con: Might not cover all zones

**Recommendation**: Start with Option B for starter zones (we know these work), use Option A for others, verify manually.

### Question 2: How Many Zones to Use?

**Minimal Approach**: One zone per level bracket (9 total zones)

- Faster to implement and test
- Less variety for bots
- Example: All 1-12 Horde bots go to Durotar

**Balanced Approach**: Multiple zones per bracket, rotate by race (current V8 approach)

- Better distribution
- More realistic
- More complex to maintain

**Full Approach**: All zones with ZoneBrackets (50+ zones)

- Maximum variety
- Closest to dynamic system
- Very complex relocation logic

**Recommendation**: Use Balanced Approach - rotate starter zones by race, use 2-3 zones per other bracket.

### Question 3: Should We Keep Validation Logging?

**Current**: Logs every teleport attempt (VERY verbose)

**Options**:

- Keep as-is: Good for debugging, floods logs
- Only log BLOCKED: Cleaner logs, see only issues
- Remove entirely: Clean logs, harder to debug later

**Recommendation**: Change to only log BLOCKED events, remove "attempting" logs once V9 is confirmed working.

---

## Known Working State

**Last Known Clean State**: N/A - Never achieved fully working state

**Best Result So Far**: V8 immediately after relocation

- All starter zones had 0 bots above level 12
- Within 1 minute, high-level bots appeared (due to wrong coordinates in script)

---

## Next Session Action Plan

1. ✅ Read this document
2. ✅ SSH to server: `ssh root@testing-azerothcore.rollet.family`
3. ✅ Back up current V8 script: `cp relocate_bots_dynamic.sh relocate_bots_v8_broken.sh`
4. ⚠️ Create V9 script with verified coordinates
5. ⚠️ Test V9 by restarting server
6. ⚠️ Check immediate post-relocation distribution
7. ⚠️ Wait 5-10 minutes, check again
8. ⚠️ Analyze logs for BLOCKED messages
9. ⚠️ If issues persist, investigate other teleport paths
10. ⚠️ Consider removing verbose "attempting" logs once working

---

## Questions to Answer

1. **Are there other teleport functions** besides `RandomTeleport()` that bypass validation?
2. **Why are there zero BLOCKED logs** when we know bots are in wrong zones?
3. **Do bots ever randomly teleport** or do they stay in their spawn zone indefinitely?
4. **What triggers a bot to attempt teleportation**? (Time interval? Event? Level up?)
5. **Should capital cities** (1519, 1537, 1637, etc.) have ZoneBracket definitions?

---

## Glossary

- **ZoneBracket**: Min/max level range defining which bot levels are appropriate for a zone
- **zone2LevelBracket**: C++ map storing ZoneBracket definitions for runtime validation
- **RandomTeleport()**: Core function that handles bot teleportation
- **locsPerLevelCache**: Cache of valid teleport locations organized by bot level
- **Relocation Script**: Bash script that runs before worldserver to place bots in correct zones
- **Validation**: Runtime check that prevents bots from teleporting to inappropriate zones

---

## Contact

**Server**: testing-azerothcore.rollet.family  
**Access**: SSH as root  
**Project**: AzerothCore with mod-playerbots

---

_End of Status Report_
