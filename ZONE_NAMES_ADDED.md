# Zone Names Implementation

## Problem

The zones list in bottop was showing "No zones found" because:

1. The `areatable_dbc` table in AzerothCore database was empty (0 rows)
2. The query was trying to LEFT JOIN this table to get zone names
3. Without zone names, zones couldn't be displayed properly

## Solution

**Hardcoded WotLK zone names directly into bottop's code.**

### Changes Made

#### 1. Added Zone Name Map (`src/btop_azerothcore.hpp`)

- Added `#include <unordered_map>`
- Created `ZONE_NAMES` constant map with ~100 common WotLK zones including:
    - Classic Eastern Kingdoms (Durotar, Elwynn Forest, Stormwind, etc.)
    - Classic Kalimdor (Mulgore, Thunder Bluff, Barrens, etc.)
    - The Burning Crusade (Hellfire Peninsula, Nagrand, Silvermoon City, etc.)
    - Wrath of the Lich King (Howling Fjord, Dragonblight, Icecrown, etc.)
    - Dungeons and Raids (Naxxramas, Karazhan, Ulduar, etc.)

- Created helper function:
    ```cpp
    inline std::string get_zone_name(int zone_id)
    ```
    Returns zone name from ID, or "Unknown Zone (ID)" if not found

#### 2. Simplified Zone Query (`src/btop_azerothcore.cpp`)

**Old query:** Complex LEFT JOIN to non-existent `areatable_dbc` table  
**New query:** Simple query that just gets zone IDs and stats:

```sql
SELECT zone, COUNT(*), MIN(level), MAX(level)
FROM characters
WHERE online = 1 AND account NOT IN (admin accounts)
GROUP BY zone
HAVING COUNT(*) >= 3
ORDER BY COUNT(*) DESC
LIMIT 15
```

- Removed dependency on `areatable_dbc` table
- Uses hardcoded `ZONE_NAMES` map for name lookup
- Added comprehensive debug logging
- Simplified from 8 columns to 4 columns returned

### Zone Name Source

Zone IDs and names sourced from:

- https://wowpedia.fandom.com/wiki/AreaId
- WotLK 3.3.5a client data
- Standard AzerothCore zone mappings

### Top Zones on Testing Server

Based on query results, most populated zones are:

1. **Durotar** (ID 14) - 99 bots
2. **Ironforge** (ID 1537) - 92 bots
3. **Dun Morogh** (ID 1) - 90 bots
4. **Azuremyst Isle** (ID 3524) - 89 bots
5. **Eversong Woods** (ID 3430) - 85 bots

### Benefits

✅ No database dependency - works even if `areatable_dbc` is empty  
✅ Faster queries - no JOIN needed  
✅ Consistent zone names across all servers  
✅ Easy to add more zones if needed  
✅ Full debug logging for troubleshooting

### Testing

Run bottop and check `~/.local/state/btop.log` for debug messages:

```
fetch_zones: Got result with X bytes
fetch_zones: Parsed zone 'Durotar' (ID: 14) with 99 bots
fetch_zones: Successfully parsed 15 zones from 15 lines
```

### Future Enhancements

- Could calculate better alignment % by querying level distribution per zone
- Could add more obscure zone IDs if bots visit them
- Could make expected min/max levels more accurate per zone
