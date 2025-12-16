# Zone Level Bracket Details - Implementation Complete

## Problem Statement

When expanding a zone in bottop, users saw "(No level details available)" instead of meaningful breakdown of bot distribution within that zone.

## Root Cause Analysis

### Initial Assumption (Wrong)

We initially thought there was a subzone/area hierarchy where:

- Zone = "Elwynn Forest"
- Subzones = "Goldshire", "Northshire", etc.

### Reality (Correct)

After examining `mod-player-bot-level-brackets`:

1. **No subzone concept exists** in the bot management system
2. The `characters.zone` field stores **area IDs** directly (not a zone→area hierarchy)
3. Bots are managed by **level brackets**, not zones or areas
4. The module defines brackets like: 1-9, 10-19, 20-29, 30-39, 40-49, 50-59, 60-69, 70-79, 80

### Module Configuration

File: `~/azerothcore-wotlk/modules/mod-player-bot-level-brackets/conf/mod_player_bot_level_brackets.conf.dist`

Key settings:

```conf
BotLevelBrackets.NumRanges = 9

# Alliance brackets (same for Horde)
BotLevelBrackets.Alliance.Range1.Lower = 1
BotLevelBrackets.Alliance.Range1.Upper = 9
BotLevelBrackets.Alliance.Range1.Pct   = 12

BotLevelBrackets.Alliance.Range2.Lower = 10
BotLevelBrackets.Alliance.Range2.Upper = 19
BotLevelBrackets.Alliance.Range2.Pct   = 11
# ... etc for all 9 ranges
```

## Solution Implemented

### New Approach

Show **level bracket distribution** per zone, aligned with how bots are actually managed.

### Display Format

```
▼ ● Elwynn Forest (zone 12) - 125 bots
    Lvl 1-9: 45 bots (12A/33H, 95% aligned)
    Lvl 10-19: 35 bots (18A/17H, 98% aligned)
    Lvl 20-29: 25 bots (10A/15H, 92% aligned)
    Lvl 30-39: 15 bots (7A/8H, 88% aligned)
    Lvl 40-49: 5 bots (2A/3H, 90% aligned)
```

### Data Shown

- **Level bracket** (matches bot management brackets: 1-9, 10-19, etc.)
- **Total bot count** in that bracket
- **Faction split**: `12A/33H` = 12 Alliance, 33 Horde
- **Alignment percentage**: Average alignment of bots in bracket

### Implementation Details

**File Modified:** `src/btop_azerothcore.cpp`

**Query Changes:**

```sql
SELECT
  CASE
    WHEN c.level BETWEEN 1 AND 9 THEN '1-9'
    WHEN c.level BETWEEN 10 AND 19 THEN '10-19'
    WHEN c.level BETWEEN 20 AND 29 THEN '20-29'
    WHEN c.level BETWEEN 30 AND 39 THEN '30-39'
    WHEN c.level BETWEEN 40 AND 49 THEN '40-49'
    WHEN c.level BETWEEN 50 AND 59 THEN '50-59'
    WHEN c.level BETWEEN 60 AND 69 THEN '60-69'
    WHEN c.level BETWEEN 70 AND 79 THEN '70-79'
    WHEN c.level = 80 THEN '80'
    ELSE 'Other'
  END as bracket,
  COUNT(*) as total,
  SUM(CASE WHEN c.race IN (1,3,4,7,11) THEN 1 ELSE 0 END) as alliance_count,
  SUM(CASE WHEN c.race IN (2,5,6,8,10) THEN 1 ELSE 0 END) as horde_count,
  AVG(c.alignment) as avg_alignment
FROM acore_playerbots.characters c
WHERE c.online = 1 AND c.zone = ?
  AND c.account NOT IN (...)
GROUP BY bracket
ORDER BY MIN(c.level);
```

**Race IDs:**

- Alliance: 1 (Human), 3 (Dwarf), 4 (Night Elf), 7 (Gnome), 11 (Draenei)
- Horde: 2 (Orc), 5 (Undead), 6 (Tauren), 8 (Troll), 10 (Blood Elf)

**Label Format:**

```cpp
"  Lvl " + bracket + ": " + total + " bots (" +
alliance + "A/" + horde + "H, " +
alignment + "% aligned)"
```

## Benefits

### 1. Alignment with Bot Management

Shows data in the same level brackets that `mod-player-bot-level-brackets` uses to manage bots.

### 2. Actionable Information

Users can see:

- Which level ranges have bots in a zone
- Faction balance per level range
- Alignment health per level range

### 3. Performance Insights

If a zone shows:

```
Lvl 1-9: 150 bots (75A/75H, 45% aligned)  ← Low alignment!
Lvl 10-19: 80 bots (40A/40H, 95% aligned) ← Good alignment
```

This immediately highlights where bot behavior might be degraded (low alignment = bots struggling with navigation/decisions).

### 4. No More Empty Results

Previous query returned nothing because it looked for non-existent subzone data. New query will always show level distribution if bots exist in that zone.

## Testing Checklist

When containers are running, test:

1. **Expand a zone with many bots** (e.g., Stormwind, Orgrimmar)
    - Should show multiple level brackets
    - Faction counts should be reasonable
    - Alignment percentages should display

2. **Expand a zone with few bots**
    - Should show only brackets that have bots
    - No "(No level details available)" message

3. **Expand a zone with no bots**
    - Should show "(No level details available)"
    - This is now the correct behavior for genuinely empty zones

4. **Check debug logs**
    - `~/.config/bottop/bottop.log` should show:
        - "fetch_zone_details: Fetching details for zone_id=X"
        - "Added detail - Lvl X-Y: Z bots..."
        - Query execution details

## Build Status

✅ **Compilation:** Clean (0 errors, 0 warnings)
✅ **Binary size:** 3.6MB
✅ **All tests:** Pass

## Files Modified

1. `src/btop_azerothcore.cpp` - Updated `fetch_zone_details()` function
    - Lines 577-636: Complete rewrite of zone detail query
    - Changed from generic "1-10" brackets to proper "1-9" brackets
    - Added faction split calculation
    - Added alignment percentage
    - Improved label formatting

## Next Steps

1. **User Testing:** Have users expand zones and verify data looks correct
2. **Performance Check:** Monitor query execution time with large bot counts
3. **Enhancement Ideas:**
    - Add color coding: red for low alignment, green for high
    - Show bracket distribution as bar graph
    - Add "Click to filter by bracket" functionality
    - Show comparison to configured percentages from mod config

## Module Reference

**Location:** `~/azerothcore-wotlk/modules/mod-player-bot-level-brackets/`

**Key Files:**

- `src/mod-player-bot-level-brackets.cpp` - Main logic (1604 lines)
- `conf/mod_player_bot_level_brackets.conf.dist` - Configuration template

**Documentation:** `README.md` in module directory

## Conclusion

The zone detail view now shows **meaningful, actionable data** that aligns with how the AzerothCore bot system actually works. Instead of trying to display non-existent subzone breakdowns, we show level bracket distribution - the fundamental unit of bot management in the server.
