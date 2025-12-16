# UI Improvements

## Changes Made - December 11, 2025

### 1. Renamed "Perf:" to "Worldserver Performance:"
**File:** `src/btop_draw.cpp` (line 1609)

**Before:**
```
Perf: 0ms
```

**After:**
```
Worldserver Performance: 0ms
```

**Rationale:** More descriptive label that clearly indicates this metric represents the AzerothCore worldserver's performance, not general system performance.

---

### 2. Move Unknown Zones to Bottom of List
**File:** `src/btop_azerothcore.cpp` (lines 509-525)

**Change:** Added sorting logic to push zones with "Unknown Zone (ID)" format to the bottom of the zone list.

**Implementation:**
- Added `#include <algorithm>` for `std::sort`
- After fetching zones from database, sort them with custom comparator:
  - Known zones appear first (sorted by total descending)
  - Unknown zones appear at bottom (also sorted by total descending)

**Rationale:** Unknown zones (zones not in our hardcoded ZONE_NAMES map) are less useful for monitoring and should not clutter the top of the list. Moving them to the bottom keeps focus on recognizable, named zones.

**Example Unknown Zone:**
```
Unknown Zone (4298) - 5 players
```

This zone ID 4298 is not in our WotLK zone database and will now appear at the bottom of the list instead of being sorted purely by player count.

---

## Build Status
✅ Clean build (0 warnings)
✅ Binary: 3.6MB
✅ Both changes tested and working

## Files Modified
1. `src/btop_draw.cpp` - Label change (1 line)
2. `src/btop_azerothcore.cpp` - Added sorting logic (13 lines) + include (1 line)

## Testing
Run bottop to verify:
```bash
cd /home/havoc/bottop
./build/bottop
```

Expected behavior:
- "Worldserver Performance:" label displays instead of "Perf:"
- Unknown zones appear at the bottom of zone list
- Known zones remain sorted by player count (highest first)
