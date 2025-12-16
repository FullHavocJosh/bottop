# Zone Navigation Fix

## Issues Fixed - December 11, 2025

### Problem 1: Hard to See Which Zone is Selected
**Issue:** When pressing 'z' to enter zone navigation mode, the selected zone was highlighted in red (using `Theme::c("selected_bg")` and `Theme::c("selected_fg")`), but this was difficult to see against the terminal background.

**Root Cause:** The theme's "selected" colors were too subtle and blended in with other UI elements.

**Solution:** Replaced subtle background highlighting with a bold selection indicator.

**Implementation:**
- Added a visible `► ` arrow indicator next to the selected zone
- Changed text color to `Theme::c("hi_fg")` (high contrast foreground) for selected zones
- When not selected, zones show `  ` (two spaces) instead of the arrow

**File Modified:** `src/btop_draw.cpp` (lines 1759-1774)

**Visual Change:**
```
Before (subtle red background):
  ● ▶ Elwynn Forest                    45      1     80    100%
  ● ▶ Durotar                          32      1     75     95%

After (bold arrow indicator):
  ● ▶ Elwynn Forest                    45      1     80    100%
► ● ▶ Durotar                          32      1     75     95%  ← Selected
  ● ▶ Stormwind City                   28      1     80    100%
```

The selected zone now has:
- `► ` arrow at the beginning of the line
- High-contrast text color
- Much more visible than before

---

### Problem 2: Dropdown Arrow Shows But No Subzones/Areas
**Issue:** Pressing `Enter`, `Space`, or `→` on a zone shows the dropdown arrow (`▼`) but no level breakdown appears below it.

**Possible Root Causes:**
1. Zone details query might be failing silently
2. Query might be returning empty results
3. Data might not be getting stored properly

**Solution:** Added comprehensive debug logging to track the issue.

**Implementation:**
- Added debug logs at every step of `fetch_zone_details()`:
  - Log when function is called with zone_id
  - Log query result length
  - Log when result is empty
  - Log each detail row as it's parsed
  - Log final count of details returned

**File Modified:** `src/btop_azerothcore.cpp` (lines 577-627)

**Debug Logging Added:**
```cpp
Logger::debug("fetch_zone_details: Fetching details for zone_id=" + std::to_string(zone_id));
Logger::debug("fetch_zone_details: Query result length=" + std::to_string(result.size()));
Logger::debug("fetch_zone_details: Empty result, returning empty details");
Logger::debug("fetch_zone_details: Added detail - " + d.label + ": " + std::to_string(d.count));
Logger::debug("fetch_zone_details: Returning " + std::to_string(details.size()) + " details");
```

---

## How Zone Details Work

When you expand a zone, bottop fetches level breakdown for players in that zone:

**Query:**
```sql
SELECT 
  CASE 
    WHEN level BETWEEN 1 AND 10 THEN '1-10'
    WHEN level BETWEEN 11 AND 20 THEN '11-20'
    ...
    ELSE '80+'
  END as bracket,
  COUNT(*) as count
FROM characters
WHERE online = 1 AND zone = <zone_id>
  AND account NOT IN (...)
GROUP BY bracket
ORDER BY MIN(level);
```

**Expected Output:**
```
  Lvl 1-10           12      26%
  Lvl 11-20           8      17%
  Lvl 21-30          15      33%
  Lvl 31-40          11      24%
```

---

## Testing

### Test Zone Navigation:
```bash
cd /home/havoc/bottop
./build/bottop

# Enable debug logging first:
# Edit ~/.config/bottop/bottop.conf
# Set: log_level = "DEBUG"
```

**Steps:**
1. Press `z` to enter zone navigation mode
2. Look for `► ` arrow next to selected zone (should be very visible now)
3. Use `↑`/`↓` to navigate zones
4. Press `Enter` or `Space` on a zone to expand it
5. Check the logs for debug output:
   ```bash
   tail -f ~/.config/bottop/bottop.log
   ```

**Expected Log Output When Expanding Zone:**
```
[DEBUG] fetch_zone_details: Fetching details for zone_id=12
[DEBUG] fetch_zone_details: Query result length=87
[DEBUG] fetch_zone_details: Added detail -   Lvl 1-10: 12
[DEBUG] fetch_zone_details: Added detail -   Lvl 11-20: 8
[DEBUG] fetch_zone_details: Returning 2 details
```

**If No Details Appear:**
- Check logs for "Empty result" message
- Verify players are actually in that zone
- Zone might have < 3 players (query filters zones with < 3 players in main zone list)
- Try expanding a different zone with more players

---

## Possible Reasons for Empty Zone Details

1. **No players in zone** - Zone list shows zones with ≥3 players, but all might be same level
2. **Query filter** - Excludes admin accounts (HAVOC, JOSHG, etc.)
3. **Timing issue** - Players might have moved zones between main query and detail query
4. **Zone ID mismatch** - Zone ID might not match (check logs for zone_id value)

---

## Navigation Controls (Reminder)

**Zone Navigation Mode (Press 'z'):**
- `↑`/`k` or `↓`/`j` - Navigate up/down through zones
- `Enter` or `Space` or `→` - Expand/collapse zone details
- `←` or `Esc` - Exit zone navigation mode

**Visual Indicators:**
- `► ` - Selected zone (bold, high contrast)
- `● ` - Zone health good (green)
- `⚠ ` - Zone health warning (red)
- `▶ ` - Zone collapsed (can expand)
- `▼ ` - Zone expanded (showing details)

---

## Build Status
✅ Clean build (0 warnings)
✅ Binary: 3.6MB
✅ Selection indicator much more visible
✅ Debug logging added for troubleshooting

## Files Modified
1. `src/btop_draw.cpp` - Improved selection indicator (arrow + high contrast)
2. `src/btop_azerothcore.cpp` - Added debug logging to zone details fetching

## Next Steps for User

**If zone details still don't show after this fix:**
1. Enable debug logging (`log_level = "DEBUG"` in config)
2. Run bottop and try expanding a zone
3. Check `~/.config/bottop/bottop.log` for debug messages
4. Share the log output to diagnose the specific issue

The selection visibility issue is definitely fixed. The zone details issue needs testing with debug logs to understand why they might be empty.
