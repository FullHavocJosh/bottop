# Zone Display Improvements

## Session 1: Single-Line Zone Display (Dec 14, 2025)

### 1. Added Total Bot Count Header

**Location:** `src/btop_draw.cpp:1926-1931, 2000-2002`

Added calculation and display of total bots across all zones at the top of the zones list.

### 2. Made Zones Single-Line Display

**Location:** `src/btop_draw.cpp:2056-2077`

- **Before:** Each zone took 2+ vertical lines
- **After:** Each zone takes exactly 1 line
- Removed expandable sections and detail lines

### 3. Fixed Zone Name Filtering

**Location:** `src/btop_draw.cpp:1933-1952`

Fixed filtering to only show matching zones and their parent containers.

---

## Session 2: Continent and Zone Name Fixes (Dec 15, 2025)

### Issues Fixed

#### 1. "Other" Continent Showing Up

**Problem**: Bots on map 609 (Ebon Hold/Death Knight starting area) were categorized as "Other" continent.

**Solution**: Updated continent mapping in `btop_azerothcore.cpp:764-778`:

- Map 609 → "Eastern Kingdoms" (Ebon Hold is located in EK)
- Map 30, 489, 529 → "Battlegrounds" (AV, WSG, AB)
- Everything else → "Instances" (instead of "Other")

**Correction Applied**: Initially mapped to "Ebon Hold" as separate continent, corrected to "Eastern Kingdoms" per lore accuracy.

#### 2. Unknown Zone (4080)

**Problem**: Zone 4080 (Isle of Conquest) showed as "Unknown Zone (4080)".

**Solution**: Added to `btop_azerothcore.hpp`:

- Zone 4080 & 4710 → "Isle of Conquest" in `ZONE_NAMES`
- Added metadata: `{4080, {"Northrend", "Isle of Conquest", 77, 80}}`

### Zone Grouping by Continent

Zones are already correctly grouped hierarchically in `btop_azerothcore.cpp:958-997`:

1. By Continent: Eastern Kingdoms → Kalimdor → Outland → Northrend → Unknown
2. By Region: Within each continent
3. By Population: Within each region (descending)

### Files Modified

1. **`src/btop_azerothcore.cpp`** (lines 764-778) - continent mapping
2. **`src/btop_azerothcore.hpp`** (lines 62-69) - ZONE_NAMES
3. **`src/btop_azerothcore.hpp`** (lines 277-280) - ZONE_METADATA

### Testing

After rebuilding, you should see:

- ✅ No more "Other" continent (replaced with specific names)
- ✅ No more "Unknown Zone (4080)"
- ✅ Zones properly grouped by continent

### Adding More Zones

If you see other unknown zones:

1. Check zone IDs:
    ```bash
    ssh root@testing-azerothcore.rollet.family "docker exec testing-ac-database mysql -u root -ppassword -D acore_characters -e \"SELECT DISTINCT zone, COUNT(*) as count FROM characters WHERE online=1 GROUP BY zone ORDER BY count DESC LIMIT 20;\" 2>/dev/null"
    ```
2. Look up zone names (WoWHead, etc.)
3. Add to both `ZONE_NAMES` and `ZONE_METADATA` in `btop_azerothcore.hpp`

## Build Status

✅ Successfully compiled (Dec 15, 2025, 23:45)

- Build time: 1m 14s
- Binary: `/home/havoc/bottop/bin/bottop`
