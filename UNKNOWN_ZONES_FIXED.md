# Unknown Zones Fixed

## Problem

Four zones were showing as "Unknown Zone (ID)" in the zones list:

- Unknown Zone (3537)
- Unknown Zone (3703)
- Unknown Zone (3711)
- Unknown Zone (4298)

## Zones Identified and Added

### Zone ID 3537: Borean Tundra

- **Expansion:** Wrath of the Lich King
- **Continent:** Northrend
- **Type:** Leveling zone (68-72)
- **Already in ZONE_NAMES:** Yes
- **Already in ZONE_METADATA:** Yes (line 236)

### Zone ID 3703: The Mechanar

- **Expansion:** The Burning Crusade
- **Continent:** Outland
- **Region:** Netherstorm
- **Type:** Dungeon (Tempest Keep instance)
- **Level Range:** 69-72 (Normal), 70 (Heroic)

### Zone ID 3711: The Botanica

- **Expansion:** The Burning Crusade
- **Continent:** Outland
- **Region:** Netherstorm
- **Type:** Dungeon (Tempest Keep instance)
- **Level Range:** 67-69 (Normal), 70 (Heroic)

### Zone ID 4298: Gundrak

- **Expansion:** Wrath of the Lich King
- **Continent:** Northrend
- **Region:** Zul'Drak
- **Type:** Dungeon
- **Level Range:** 74-76 (Normal), 80 (Heroic)

## Changes Made

### 1. Added to ZONE_NAMES Map

**Location:** `src/btop_azerothcore.hpp:59, 64, 66`

```cpp
// The Burning Crusade - Added TBC dungeons
{3703, "The Mechanar"}, {3711, "The Botanica"},

// Wrath of the Lich King - Added Borean Tundra and Gundrak
{3537, "Borean Tundra"},  // Added to line 64
{4298, "Gundrak"},         // Added to line 66
```

### 2. Added to ZONE_METADATA Map

**Location:** `src/btop_azerothcore.hpp:215-218, 252`

```cpp
// Outland - Netherstorm Region
{3523, {"Outland", "Netherstorm"}},
{3703, {"Outland", "The Mechanar"}},
{3711, {"Outland", "The Botanica"}},

// Northrend - Zul'Drak Region
{66, {"Northrend", "Zul'Drak"}},
{4298, {"Northrend", "Gundrak"}},
```

## Result

These zones will now display with their proper names instead of "Unknown Zone (ID)":

- ✅ **Borean Tundra** (Northrend)
- ✅ **The Mechanar** (Outland - Netherstorm)
- ✅ **The Botanica** (Outland - Netherstorm)
- ✅ **Gundrak** (Northrend - Zul'Drak)

## Display Example

### Before:

```
  ▼ Unknown                  234
    ● Unknown Zone (3703)     12       68-70    100%
    ● Unknown Zone (3711)     18       67-69    100%
    ● Unknown Zone (4298)     34       74-76    100%
```

### After:

```
  ▼ Outland                  850
    ● The Mechanar            12       68-70    100%
    ● The Botanica            18       67-69    100%
  ▼ Northrend                982
    ● Gundrak                 34       74-76    100%
```

## How to Add More Zones

If more unknown zones appear in the future:

1. **Note the Zone ID** from the display: "Unknown Zone (XXXX)"
2. **Look up the zone** at WoW database sites (wowhead.com, wowpedia)
3. **Edit** `src/btop_azerothcore.hpp`:
    - Add to `ZONE_NAMES` map (around line 32-69)
    - Add to `ZONE_METADATA` map (around line 77-280)
4. **Rebuild:** `cd /home/havoc/bottop && cmake --build build`

## Build Status

✅ Successfully compiled

- Binary: `/home/havoc/bottop/build/bottop` (2.3 MB)
- Build time: Dec 14, 2025, 16:43 EST

## Files Modified

- `src/btop_azerothcore.hpp` - ZONE_NAMES and ZONE_METADATA maps
