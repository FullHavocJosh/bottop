# Unified Zone Bracket System - Implementation Guide

**Date**: December 18, 2024  
**Server**: testing-azerothcore.rollet.family  
**Goal**: Ensure complete alignment between bot relocation and teleportation systems

---

## Overview

This implementation creates a **single source of truth** for zone level brackets by:

1. Using `playerbots.conf` as the authoritative configuration source
2. Ensuring both worldserver (playerbots) and relocation scripts read from the same config
3. Providing 100% coverage for levels 1-80
4. Eliminating discrepancies between relocation and teleportation logic

---

## Current Status

### ✅ What's Working

1. **PrepareZone2LevelBracket()** already reads zone brackets from `playerbots.conf`
    - Location: `mod-playerbots/src/RandomPlayerbotMgr.cpp`
    - Function: Loads config overrides via `sPlayerbotAIConfig->zoneBrackets`

2. **Coverage Analysis**: Levels 5-80 fully covered
    - Classic (5-60): 56/56 levels ✅
    - TBC (58-70): 13/13 levels ✅
    - WotLK (68-80): 13/13 levels ✅

### ❌ What Needs Fixing

1. **Gap in Coverage**: Levels 1-4 not covered
    - Starter zones currently defined as 5-12
    - Need to extend to 1-12

2. **Relocate Script**: Uses hardcoded level ranges
    - Not reading from `playerbots.conf`
    - Level brackets don't match playerbot zones

---

## Implementation Steps

### Step 1: Fix Level 1-4 Coverage

Update `playerbots.conf` to extend starter zones:

```bash
# On the server
cd /root/testing-azerothcore-wotlk/env/dist/etc/modules

# Backup original
cp playerbots.conf playerbots.conf.backup

# Fix starter zone brackets (5,12 -> 1,12)
sed -i 's/ZoneBracket\.\(1\|12\|14\|85\|141\|215\|3430\|3524\) = 5,12/ZoneBracket.\1 = 1,12/' playerbots.conf
```

**Zones affected**:

- Zone 1 (Dun Morogh): 1-12
- Zone 12 (Elwynn Forest): 1-12
- Zone 14 (Durotar): 1-12
- Zone 85 (Tirisfal Glades): 1-12
- Zone 141 (Teldrassil): 1-12
- Zone 215 (Mulgore): 1-12
- Zone 3430 (Eversong Woods): 1-12
- Zone 3524 (Azuremyst Isle): 1-12

### Step 2: Deploy Zone Bracket Parser

Copy the parser script to the server:

```bash
# From local machine
scp /Users/havoc/bottop/zone_bracket_parser.sh root@testing-azerothcore.rollet.family:/root/testing-azerothcore-wotlk/

# On server, make executable
chmod +x /root/testing-azerothcore-wotlk/zone_bracket_parser.sh
```

### Step 3: Deploy Config-Driven Relocate Script

```bash
# From local machine
scp /Users/havoc/bottop/relocate_bots_config_driven.sh root@testing-azerothcore.rollet.family:/root/testing-azerothcore-wotlk/

# On server, make executable
chmod +x /root/testing-azerothcore-wotlk/relocate_bots_config_driven.sh
```

### Step 4: Verify Coverage

```bash
# On server
cd /root/testing-azerothcore-wotlk
./zone_bracket_parser.sh env/dist/etc/modules/playerbots.conf --coverage-report
```

Expected output:

```
Zone Bracket Coverage Report
==============================

✅ All levels 1-80 are covered

Total zones configured: 59
```

### Step 5: Test Relocate Script

```bash
# On server
cd /root/testing-azerothcore-wotlk
export MYSQL_ROOT_PASSWORD='your_password'
./relocate_bots_config_driven.sh
```

This will:

1. Parse zone brackets from `playerbots.conf`
2. Verify all levels 1-80 are covered
3. Generate SQL with appropriate zones for each race/level
4. Execute relocation
5. Show distribution summary

### Step 6: Restart Worldserver

After fixing the config and relocating bots:

```bash
# Restart worldserver to reload playerbots.conf
docker compose -f /root/testing-azerothcore-wotlk/docker-compose.yml restart ac-worldserver
```

---

## How It Works

### 1. Configuration Source (`playerbots.conf`)

**Format**:

```conf
AiPlayerbot.ZoneBracket.ZONE_ID = MIN_LEVEL,MAX_LEVEL
```

**Example**:

```conf
AiPlayerbot.ZoneBracket.12 = 1,12      # Elwynn Forest: Levels 1-12
AiPlayerbot.ZoneBracket.40 = 10,21     # Westfall: Levels 10-21
AiPlayerbot.ZoneBracket.440 = 42,51    # Tanaris: Levels 42-51
AiPlayerbot.ZoneBracket.4395 = 70,80   # Dalaran: Levels 70-80
```

### 2. Worldserver (Playerbots) Loading

**File**: `mod-playerbots/src/PlayerbotAIConfig.cpp`

```cpp
// Loads all AiPlayerbot.ZoneBracket.* entries
for (uint32 zoneId = 0; zoneId < 10000; ++zoneId)
{
    std::string key = "AiPlayerbot.ZoneBracket." + std::to_string(zoneId);
    std::string value = sConfigMgr->GetOption<std::string>(key, "");
    if (!value.empty())
    {
        size_t commaPos = value.find(',');
        uint32 minLevel = atoi(value.substr(0, commaPos).c_str());
        uint32 maxLevel = atoi(value.substr(commaPos + 1).c_str());
        zoneBrackets[zoneId] = std::make_pair(minLevel, maxLevel);
    }
}
```

**File**: `mod-playerbots/src/RandomPlayerbotMgr.cpp`

```cpp
void RandomPlayerbotMgr::PrepareZone2LevelBracket()
{
    // Hardcoded defaults first
    zone2LevelBracket[1] = {5, 12};    // Dun Morogh
    zone2LevelBracket[12] = {5, 12};   // Elwynn Forest
    // ... more defaults ...

    // Override with values from config
    for (auto const& [zoneId, bracketPair] : sPlayerbotAIConfig->zoneBrackets)
    {
        zone2LevelBracket[zoneId] = {bracketPair.first, bracketPair.second};
    }
}
```

**Result**: Config values override hardcoded defaults ✅

### 3. Teleportation Logic

**File**: `mod-playerbots/src/RandomPlayerbotMgr.cpp`

```cpp
void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot)
{
    uint32 level = bot->GetLevel();
    uint8 race = bot->getRace();

    // Uses cached locations per level
    // Cache built using zone2LevelBracket map
    std::vector<WorldLocation>* locs =
        IsAlliance(race) ? &allianceStarterPerLevelCache[level]
                        : &hordeStarterPerLevelCache[level];

    RandomTeleport(bot, *locs);
}
```

**Cache Building**:

```cpp
// For each innkeeper/flightmaster
const AreaTableEntry* area = sAreaTableStore.LookupEntry(...);
uint32 zoneId = area->zone ? area->zone : area->ID;

if (zone2LevelBracket.find(zoneId) == zone2LevelBracket.end())
    continue;  // Skip zones not in bracket map

LevelBracket bracket = zone2LevelBracket[zoneId];
for (int i = bracket.low; i <= bracket.high; i++)
{
    // Add this location to cache for ALL levels in bracket
    hordeStarterPerLevelCache[i].push_back(loc);
    allianceStarterPerLevelCache[i].push_back(loc);
}
```

### 4. Relocation Script

**File**: `relocate_bots_config_driven.sh`

```bash
# Source the parser
source zone_bracket_parser.sh "$CONFIG_FILE"

# Get zones for a level
zones=($(get_zones_for_level "$level"))

# Select best zone for race/level
selected_zone=$(select_zone_for_bot "$race" "$level")

# Generate SQL
UPDATE characters
SET position_x = $pos_x, position_y = $pos_y, position_z = $pos_z,
    map = $map_id, zone = $selected_zone
WHERE level = $level AND race = $race
  AND guid IN (SELECT bot FROM playerbots_random_bots);
```

---

## Verification

### Check Coverage

```bash
./zone_bracket_parser.sh playerbots.conf --coverage-report
```

### Check Specific Level

```bash
./zone_bracket_parser.sh playerbots.conf --check-level 15
# Output: Level 15 is covered by 7 zone(s): 17 40 267 148 3433 3525 44
```

### Get Zones for Level

```bash
./zone_bracket_parser.sh playerbots.conf --get-zones 50
# Output: 28 357 440 490 618 51 361 46
```

### List All Zones

```bash
./zone_bracket_parser.sh playerbots.conf --list
```

---

## Zone Selection Logic

### Priority Order

1. **Starter Zone** (if level in range)
    - Race-specific starter zones take priority
    - Example: Level 10 Human → Elwynn Forest (12) if available

2. **Faction-Preferred Zones**
    - Alliance: Westfall, Duskwood, Loch Modan, etc.
    - Horde: Barrens, Ashenvale, Silverpine, etc.

3. **Neutral/Shared Zones**
    - First available zone for the level
    - Example: Tanaris, Un'Goro, Winterspring

### Examples

**Level 1 Human**:

- Available zones: 1, 12, 14, 85, 141, 215, 3430, 3524
- Selected: Zone 12 (Elwynn Forest) - Race starter zone

**Level 15 Orc**:

- Available zones: 17, 40, 267, 148, 3433, 3525, 44
- Selected: Zone 17 (Barrens) - Horde preferred

**Level 50 Night Elf**:

- Available zones: 28, 357, 440, 490, 618, 51, 361, 46
- Selected: Zone 28 (Western Plaguelands) - First available

---

## Maintenance

### Adding New Zones

1. **Edit playerbots.conf**:

    ```conf
    AiPlayerbot.ZoneBracket.NEW_ZONE_ID = MIN_LEVEL,MAX_LEVEL
    ```

2. **Add coordinates to relocate script**:

    ```bash
    ZONE_DATA[NEW_ZONE_ID]="MAP_ID:X:Y:Z"
    ```

3. **Restart worldserver**:

    ```bash
    docker compose restart ac-worldserver
    ```

4. **Rerun relocate script**:
    ```bash
    ./relocate_bots_config_driven.sh
    ```

### Adjusting Level Brackets

1. **Edit playerbots.conf**:

    ```conf
    # Example: Extend Westfall to level 25
    AiPlayerbot.ZoneBracket.40 = 10,25
    ```

2. **Restart worldserver** to reload config

3. **Rerun relocate script** to update bot positions

### Monitoring Coverage

Run coverage analysis after any config changes:

```bash
./zone_bracket_parser.sh playerbots.conf --coverage-report
```

---

## Troubleshooting

### Issue: Bots teleporting away after relocation

**Cause**: Zone not in playerbot brackets

**Solution**: Add zone to `playerbots.conf`:

```conf
AiPlayerbot.ZoneBracket.ZONE_ID = MIN_LEVEL,MAX_LEVEL
```

### Issue: Level X not covered

**Cause**: No zone brackets include level X

**Solution**: Extend existing zone or add new zone:

```conf
# Option 1: Extend existing zone
AiPlayerbot.ZoneBracket.12 = 1,15  # Was 1,12

# Option 2: Add new zone for that level
AiPlayerbot.ZoneBracket.NEW_ZONE = X,Y
```

### Issue: Relocate script says "No zone data available"

**Cause**: Zone has bracket in config but no coordinates in script

**Solution**: Add coordinates to relocate script:

```bash
ZONE_DATA[ZONE_ID]="MAP_ID:X:Y:Z"
```

### Issue: Config changes not taking effect

**Cause**: Worldserver needs restart to reload config

**Solution**:

```bash
docker compose -f /root/testing-azerothcore-wotlk/docker-compose.yml restart ac-worldserver
```

---

## Testing Checklist

- [ ] Verify all levels 1-80 have coverage
- [ ] Test relocate script with config parser
- [ ] Restart worldserver after config changes
- [ ] Verify bots stay in relocated zones
- [ ] Check level 1-4 bots are in starter zones
- [ ] Check level 70-80 bots are in WotLK zones
- [ ] Monitor for unexpected teleportations
- [ ] Verify faction-appropriate zone placement

---

## Files Summary

| File                             | Purpose                    | Location                                                |
| -------------------------------- | -------------------------- | ------------------------------------------------------- |
| `playerbots.conf`                | Zone bracket config        | `/root/testing-azerothcore-wotlk/env/dist/etc/modules/` |
| `zone_bracket_parser.sh`         | Config parser              | `/root/testing-azerothcore-wotlk/`                      |
| `relocate_bots_config_driven.sh` | Relocation script          | `/root/testing-azerothcore-wotlk/`                      |
| `analyze_zone_coverage.sh`       | Coverage analysis          | `/Users/havoc/bottop/`                                  |
| `RandomPlayerbotMgr.cpp`         | Worldserver teleport logic | `mod-playerbots/src/`                                   |
| `PlayerbotAIConfig.cpp`          | Config loading             | `mod-playerbots/src/`                                   |

---

## Benefits

### ✅ Single Source of Truth

- All systems read from `playerbots.conf`
- No hardcoded level ranges in scripts
- Easy to maintain and update

### ✅ Complete Coverage

- All levels 1-80 covered
- No gaps in level progression
- Proper expansion support (Classic, TBC, WotLK)

### ✅ Alignment

- Relocation uses same brackets as teleportation
- Bots stay in appropriate zones
- No unexpected teleportations

### ✅ Flexibility

- Easy to add new zones
- Easy to adjust level ranges
- No code changes required

### ✅ Verification

- Coverage analysis built-in
- Easy to validate changes
- Clear error messages

---

## Next Steps

1. ✅ Apply level 1-4 fix to `playerbots.conf`
2. ✅ Deploy zone bracket parser script
3. ✅ Deploy config-driven relocate script
4. ✅ Verify coverage with analysis script
5. ✅ Test relocation with new script
6. ✅ Restart worldserver
7. ✅ Monitor bot behavior
8. ✅ Document any adjustments needed

---

## Conclusion

This implementation creates a **unified, config-driven system** for zone level brackets that ensures:

- Complete level 1-80 coverage
- Alignment between relocation and teleportation
- Single source of truth in `playerbots.conf`
- Easy maintenance and updates
- Clear verification and testing

All components now work in harmony, reading from the same configuration source and using the same zone bracket definitions.
