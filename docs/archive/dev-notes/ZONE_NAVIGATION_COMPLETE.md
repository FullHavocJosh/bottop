# Zone Navigation Feature - Implementation Complete

## Summary

Implemented interactive zone navigation with expansion/collapse functionality to show level distribution details.

## Changes Made

### 1. Fixed Missing Include (btop_draw.hpp:23)

- Added `#include <set>` to support `std::set<size_t> expanded_zones`

### 2. Data Structures (btop_azerothcore.hpp)

- Added `ZoneDetail` struct (lines 126-133):
    - Stores level range (min_level, max_level)
    - Player count and percentage for each level bracket
- Updated `Zone` struct (lines 135-145):
    - Added `zone_id` field
    - Added `vector<ZoneDetail> details` for level distribution
- Added `fetch_zone_details(int zone_id)` to Query class (line 209)

### 3. Backend Implementation (btop_azerothcore.cpp)

- **Store zone_id** (lines 485-487):
    ```cpp
    zone.zone_id = zone_id;
    ```
- **Implemented fetch_zone_details()** (lines 564-617):
    - Queries character level distribution per zone
    - Groups by 10-level brackets (1-10, 11-20, etc.)
    - Returns vector of ZoneDetail with counts and percentages

### 4. UI State Variables (btop_draw.cpp:2404-2411)

```cpp
int selected_zone = 0;              // Currently selected zone index
bool zone_selection_active = false; // Navigation mode active
std::set<size_t> expanded_zones;    // Tracks which zones are expanded
```

### 5. Drawing Logic (btop_draw.cpp:2630-2715)

Completely rewrote zone list rendering:

- **Selection highlighting**: Different background/foreground colors for selected zone
- **Expand/collapse icons**: `▶` (collapsed) / `▼` (expanded)
- **Level distribution display**: Shows breakdown when zone is expanded:
    - Level range (e.g., "Level 1-10")
    - Player count
    - Percentage of zone population
- **Auto-scroll**: Keeps selected zone visible
- **Help text**: Shows navigation keys at bottom when active

### 6. Input Handling (btop_input.cpp:283-326)

Added keyboard controls:

- `z` - Enter zone navigation mode
- `↑` / `k` - Move selection up
- `↓` / `j` - Move selection down
- `Enter` / `→` / `Space` - Toggle zone expansion
- `Esc` / `←` - Exit navigation mode

## How to Test

### 1. Start bottop

```bash
cd /home/havoc/bottop
./build/bottop
```

### 2. Navigate to Zone View

The AzerothCore box should show a list of zones with player counts.

### 3. Enter Navigation Mode

Press `z` - you should see:

- Help text appear at bottom: "↑↓:Navigate Enter/→:Expand Esc/←:Exit Space:Toggle"
- First zone highlighted with selection colors

### 4. Test Navigation

- Press `↓` or `j` - Selection moves down
- Press `↑` or `k` - Selection moves up
- Auto-scroll should keep selection visible

### 5. Test Expansion

- Press `Enter`, `→`, or `Space` on a zone:
    - Icon changes from `▶` to `▼`
    - Level distribution details appear below zone
    - Shows brackets like:
        ```
          Level 1-10      50    35%
          Level 11-20     80    55%
          Level 21-30     15    10%
        ```
- Press again to collapse

### 6. Exit Navigation

- Press `Esc` or `←` - Returns to normal view
- Selection highlighting removed
- Help text disappears

## Expected Behavior

### Visual Elements

1. **Normal View**:
    - Zone list shows: `● ▶ Orgrimmar     150    1   80  89%`
    - Help text: "Press 'z' to navigate zones"

2. **Navigation Mode**:
    - Selected zone has highlighted background
    - Help shows: "↑↓:Navigate Enter/→:Expand Esc/←:Exit Space:Toggle"

3. **Expanded Zone**:
    - Icon changes to `▼`
    - Indented level distribution shown below
    - Each bracket shows: level range, count, percentage

### Database Query

When zone is expanded, runs:

```sql
SELECT
    MIN(level) as min_level,
    MAX(level) as max_level,
    COUNT(*) as count
FROM characters
WHERE map = ? AND zone = ?
GROUP BY FLOOR((level - 1) / 10)
ORDER BY min_level
```

## Files Modified

1. `src/btop_draw.hpp` - Added `#include <set>`
2. `src/btop_draw.cpp` - Zone rendering with selection/expansion (lines 2404-2715)
3. `src/btop_input.cpp` - Keyboard input handling (lines 283-326)
4. `src/btop_azerothcore.hpp` - Data structures (lines 126-145, 209)
5. `src/btop_azerothcore.cpp` - Query methods (lines 485-487, 564-617)

## Build Status

✅ **Build successful** - No errors, no warnings

## Known Limitations

1. Zone details fetched on-demand (may have slight delay first time)
2. Expanded zones collapse when leaving navigation mode
3. Scroll position resets when re-entering navigation mode

## Next Steps (Future Enhancements)

1. Cache zone details to avoid re-fetching
2. Persist expanded state when re-entering navigation mode
3. Add faction breakdown within level brackets
4. Show class distribution per zone
5. Add filtering by level range or faction
