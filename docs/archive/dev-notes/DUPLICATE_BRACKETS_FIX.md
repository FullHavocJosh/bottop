# Duplicate Level Brackets Fix

## Problem

The bot distribution pane was showing duplicate level brackets: "70-79, 80, 71, 79, 80" instead of a clean list like "70-79, 80".

## Root Cause

The issue was in the config parsing logic in `src/btop_azerothcore.cpp` (function `load_expected_values()`).

When parsing bracket definitions from the remote worldserver.conf file:

1. The code read ALL Range definitions from the config file (Range1, Range2, Range3, etc.)
2. If the config file had duplicate or overlapping range definitions (e.g., both a "70-79" range and separate "71", "79" ranges), they would all be loaded
3. These overlapping brackets would then generate duplicate SQL CASE statements
4. The display would show multiple entries for the same levels

## Solution

Added overlap detection in the bracket loading code (lines 2073-2101 in btop_azerothcore.cpp):

```cpp
// Check for overlaps with existing brackets
bool overlaps = false;
for (const auto& existing : expected_values.bracket_definitions) {
    // Check if ranges overlap: two ranges DON'T overlap only if one ends before the other starts
    if (!(max_level < existing.min_level || min_level > existing.max_level)) {
        debug_log << "    WARNING: Bracket " << min_level << "-" << max_level
                  << " overlaps with existing bracket " << existing.min_level << "-" << existing.max_level
                  << ", skipping!" << std::endl;
        overlaps = true;
        break;
    }
}

if (overlaps) {
    Logger::error("load_expected_values: Skipping overlapping bracket ...");
    continue;  // Skip this bracket
}
```

## Changes Made

### src/btop_azerothcore.cpp

1. **Added raw bracket data debug logging** (lines ~2053-2063):
    - Shows the parsed Lower, Upper, and Pct values for each Range before processing
    - Helps diagnose config file issues

2. **Added overlap detection** (lines ~2081-2098):
    - Checks each new bracket against existing brackets
    - Skips brackets that overlap with already-loaded brackets
    - Logs warnings about skipped brackets

3. **Added final bracket debug output** (lines ~2106-2111):
    - Shows all loaded brackets with their range strings
    - Helps verify the fix is working

## Debug Logs

The fix generates debug logs in `/tmp/load_expected_values_debug.log` with:

- Raw parsed bracket data from config file
- Overlap detection warnings
- Final loaded bracket list

## Testing

To test the fix:

1. Run bottop and connect to the AzerothCore server
2. Check `/tmp/load_expected_values_debug.log` for:
    - `=== RAW PARSED BRACKET DATA ===` section showing config values
    - Any "WARNING: Bracket X-Y overlaps" messages
    - `=== LOADED BRACKETS ===` section showing final brackets
3. Verify the bot distribution pane shows clean, non-duplicate brackets

## Expected Behavior

After the fix:

- Bot distribution should show clean level brackets: "1-9, 10-19, ..., 70-79, 80"
- No duplicate entries like "71", "79" appearing separately
- Config file issues (overlapping ranges) are logged but don't break the display

## Config File Fix (Optional)

If the config file has overlapping ranges, you may want to clean them up by:

1. SSH to the server
2. Edit the worldserver.conf file
3. Remove duplicate BotLevelBrackets.Alliance.RangeX definitions
4. Ensure each level appears in only one bracket

Example correct configuration:

```conf
BotLevelBrackets.Alliance.Range1.Lower = 1
BotLevelBrackets.Alliance.Range1.Upper = 9
BotLevelBrackets.Alliance.Range1.Pct = 5.0

BotLevelBrackets.Alliance.Range2.Lower = 10
BotLevelBrackets.Alliance.Range2.Upper = 19
BotLevelBrackets.Alliance.Range2.Pct = 6.0

# ... continue for all brackets ...

BotLevelBrackets.Alliance.Range10.Lower = 70
BotLevelBrackets.Alliance.Range10.Upper = 79
BotLevelBrackets.Alliance.Range10.Pct = 16.0

BotLevelBrackets.Alliance.Range11.Lower = 80
BotLevelBrackets.Alliance.Range11.Upper = 80
BotLevelBrackets.Alliance.Range11.Pct = 11.0
```

## Files Modified

- `src/btop_azerothcore.cpp` - Added overlap detection and debug logging

## Status

✅ Fix implemented and compiled
⏳ Awaiting user testing to verify the duplicate brackets are gone
