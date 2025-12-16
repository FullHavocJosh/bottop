# Testing Guide - Zone Navigation Feature

## Build Status

✅ **Build Successful** - No compilation errors

## Feature Overview

Interactive zone navigation with expansion/collapse to show player level distribution per zone.

---

## How to Test

### Prerequisites

- AzerothCore database running and configured in bottop
- At least a few characters in different zones/levels in the database
- Terminal with TTY support

### Step 1: Launch bottop

```bash
cd /home/havoc/bottop
./build/bottop
```

### Step 2: Initial View

You should see the AzerothCore box with:

- **Top section**: Performance graph (last 90 seconds)
- **Bottom section**: Zone list showing:
    - Health indicator (`●` green = healthy, `⚠` yellow = issue)
    - Zone name (e.g., "Orgrimmar", "Stormwind City")
    - Total players
    - Min/Max level
    - Faction alignment percentage
- **Help text** at bottom: "Press 'z' to navigate zones"

Example zone line:

```
● ▶ Orgrimmar           150      1     80    89%
```

---

## Testing Zone Navigation

### Test 1: Enter Navigation Mode

**Action**: Press `z`

**Expected Result**:

- First zone gets highlighted background (selection colors)
- Help text changes to: "↑↓:Navigate Enter/→:Expand Esc/←:Exit Space:Toggle"
- Expand icon `▶` visible before each zone name

### Test 2: Navigate Up/Down

**Actions**:

- Press `↓` or `j` (vim style)
- Press `↑` or `k` (vim style)

**Expected Result**:

- Selection highlight moves to next/previous zone
- If you scroll beyond visible area, the list auto-scrolls
- Scroll indicator updates (e.g., "Showing 1-15 of 50")

### Test 3: Expand a Zone

**Action**: Select a zone and press `Enter`, `→`, or `Space`

**Expected Result**:

- Expand icon changes from `▶` to `▼`
- Level distribution details appear below the zone:
    ```
    ● ▼ Orgrimmar           150      1     80    89%
        Level 1-10           35              23%
        Level 11-20          50              33%
        Level 71-80          65              43%
    ```
- Each detail line shows:
    - Level range (indented)
    - Player count
    - Percentage of zone population

### Test 4: Collapse a Zone

**Action**: Press `Enter`, `→`, or `Space` again on an expanded zone

**Expected Result**:

- Icon changes from `▼` back to `▶`
- Level distribution details disappear
- Zone returns to single-line display

### Test 5: Multiple Expansions

**Action**: Expand multiple zones

**Expected Result**:

- Multiple zones can be expanded simultaneously
- Each maintains its own expansion state
- Navigation still works smoothly
- Auto-scroll adjusts for expanded content

### Test 6: Exit Navigation Mode

**Action**: Press `Esc` or `←`

**Expected Result**:

- Selection highlighting disappears
- Help text returns to: "Press 'z' to navigate zones"
- Expanded zones remain expanded (visible but not interactive)
- Normal input controls restored

---

## Edge Cases to Test

### Empty Zone

**Setup**: Zone with no players
**Expected**: Zone shows "0" total, expansion shows no details or "No players"

### Single Level Zone

**Setup**: All players in zone are same level
**Expected**: Expansion shows single level bracket

### New Character Zone (Level 1-5)

**Setup**: Starter zone with low-level characters
**Expected**: Shows "Level 1-10" bracket with proper counts

### Max Level Zone

**Setup**: Zone with only level 80 characters (WotLK)
**Expected**: Shows "Level 71-80" bracket

### Long Zone Names

**Setup**: Zone with long name (e.g., "The Culling of Stratholme")
**Expected**: Name is truncated or wrapped properly, alignment maintained

### Rapid Navigation

**Action**: Hold down arrow key
**Expected**: Selection scrolls smoothly, no crashes, no lag

### Rapid Expand/Collapse

**Action**: Rapidly press Enter multiple times
**Expected**: Zone toggles cleanly, no visual glitches

---

## Database Query Verification

When a zone is expanded, bottop runs this query:

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

**Manual Verification**:

```sql
-- Find a zone with players
SELECT zone, COUNT(*) as total FROM characters GROUP BY zone LIMIT 5;

-- Check level distribution for zone 1637 (Orgrimmar)
SELECT
    FLOOR((level - 1) / 10) * 10 + 1 as min_level,
    FLOOR((level - 1) / 10) * 10 + 10 as max_level,
    COUNT(*) as count,
    ROUND(COUNT(*) * 100.0 / SUM(COUNT(*)) OVER(), 1) as percent
FROM characters
WHERE zone = 1637
GROUP BY FLOOR((level - 1) / 10)
ORDER BY min_level;
```

---

## Performance Testing

### Large Zone Lists (50+ zones)

**Expected**:

- Navigation remains responsive
- Scroll indicator accurate
- No lag when expanding/collapsing

### High Player Count Zones (100+ players)

**Expected**:

- Query completes quickly (< 100ms)
- Details render without delay
- Percentages add up to ~100%

### Frequent Data Updates

**Expected**:

- Expanded zones update if player distribution changes
- Selection remains stable during updates
- No flickering or visual artifacts

---

## Known Limitations

1. **Zone details cached per session**: If you expand a zone, the details are cached until bottop restarts
2. **Expansion state resets**: When exiting and re-entering navigation mode, expanded zones collapse
3. **No persistence**: Zone selection position resets when re-entering navigation mode
4. **Map dependency**: Zone IDs are map-specific; same zone on different continents may show separately

---

## Troubleshooting

### "No zones found"

- **Cause**: No characters in database, or database connection issue
- **Fix**: Check database credentials, verify characters table has data

### Zone expansion shows no details

- **Cause**: Query failed or zone has no players
- **Fix**: Check database connectivity, verify zone_id is correct

### Selection doesn't move

- **Cause**: Only one zone in list, or at end of list
- **Fix**: Normal behavior; can't navigate beyond bounds

### Crashes when pressing 'z'

- **Cause**: Database connection lost
- **Fix**: Restart bottop, verify database is running

### Visual glitches / overlapping text

- **Cause**: Terminal too small or font rendering issue
- **Fix**: Resize terminal to at least 80x24, use monospace font

---

## Success Criteria

✅ All navigation keys work as expected
✅ Zone expansion shows accurate level distribution
✅ Multiple zones can be expanded simultaneously  
✅ Auto-scroll keeps selection visible
✅ No crashes or hangs during normal operation
✅ Performance remains smooth with 50+ zones
✅ Query returns results in < 100ms
✅ Visual elements render correctly
✅ Help text appears at appropriate times

---

## Next Steps After Testing

If all tests pass:

1. ✅ Feature complete
2. Consider adding caching for zone details
3. Consider persisting expansion state
4. Consider adding faction filter
5. Consider adding class distribution view

If issues found:

1. Document specific failure case
2. Check logs for error messages
3. Verify database schema matches expectations
4. Test with different terminal sizes
5. Profile performance with large datasets
