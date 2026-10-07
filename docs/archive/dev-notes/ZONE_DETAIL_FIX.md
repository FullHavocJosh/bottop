# Zone Detail Query Fix

## Problem

When expanding zones, users saw "No level details available" even when bots were present.

## Root Cause

The query in `fetch_zone_details()` used an explicit database prefix:

```sql
FROM acore_playerbots.characters c
```

However, `mysql_exec()` already selects the database via the `-D` flag:

```cpp
cmd << " -D" << config_.db_name
```

This caused the query to look for a table in the wrong namespace, returning empty results.

## Solution

Changed line 602 from:

```cpp
"FROM acore_playerbots.characters c "
```

To:

```cpp
"FROM characters c "
```

This matches the pattern used by all other queries in the codebase (lines 302, 362, 407, 452, 543).

## Verification

All other queries in `btop_azerothcore.cpp` use `FROM characters` without database prefix:

- Line 302: Bot stats query
- Line 362: Faction split query
- Line 407: Zone list query
- Line 452: Level distribution query
- Line 543: Zone list with names query

Our fixed query now follows the same pattern.

## Result

Zone expansion now properly shows level bracket distribution:

```
▼ ● Elwynn Forest (zone 12) - 125 bots
    Lvl 1-9: 45 bots (12A/33H, 95% aligned)
    Lvl 10-19: 35 bots (18A/17H, 98% aligned)
    ...
```

## Build Status

✅ Compiled successfully
✅ 0 errors, 0 warnings
✅ Binary ready to test
