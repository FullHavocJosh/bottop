# Level Distribution Monitoring with Arrow Indicators

## Feature Implementation

Added visual indicators to the bot distribution pane showing whether each level bracket is above or below the expected distribution configured on the AzerothCore server.

## Design

### Display Format

```
Levels:
1-9         ▼  5%   (below target - red/orange arrow)
10-19          9%   (within tolerance - no arrow)
20-29       ▲ 14%   (above target - green arrow)
...
```

### Arrow Logic

Between the level bracket name and percentage, an arrow appears with color:

- **Green ▲** (Theme: "available") - Actual % is >3% **above** expected %
    - Indicates this bracket is over-represented in the bot population
- **Red/Orange ▼** (Theme: "download") - Actual % is >3% **below** expected %
    - Indicates this bracket is under-represented in the bot population
- **No Arrow** (2 spaces) - Actual % is within ±3% of expected %
    - Indicates bracket distribution is on target

### Tolerance Threshold

**±3% deviation** from expected before showing arrows:

- Expected: 10%, Actual: 7% → No arrow (within tolerance)
- Expected: 10%, Actual: 5% → Red ▼ (>3% below)
- Expected: 10%, Actual: 14% → Green ▲ (>3% above)

## Expected Distribution Source

Expected percentages come from the server's bracket configuration file:

- Path: `/azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf`
- Format: `BotLevelBrackets.Alliance.Range1.Pct = 5` (example)
- Loaded at bottop startup via `load_expected_values()`

For the 11-bracket configuration:

```
1-9:    5%
10-19:  6%
20-29: 10%
30-39:  9%
40-49:  8%
50-59:  6%
60:    12%
61-69:  7%
70:    10%
71-79: 16%
80:    11%
```

## Use Cases

### 1. Monitoring Bot Creation

Quick visual check if the bot creation is following the configured distribution:

- All brackets showing arrows → Major distribution imbalance
- Few arrows → Minor variations, mostly on target
- No arrows → Perfect distribution alignment

### 2. Debugging Bot Configuration

If many brackets show red ▼:

- Bot creation may be failing for those level ranges
- Level bracket percentages may need adjustment
- Max bot count may be too low

If many brackets show green ▲:

- Bot creation is exceeding targets for those ranges
- Other brackets likely showing red ▼ to compensate

### 3. Identifying Trends

- Consistent green ▲ at high levels → Bots leveling up faster than new ones created
- Consistent red ▼ at low levels → Not enough new bot creation
- Spread across ranges → Random variation within tolerance

## Implementation Details

**File:** `src/btop_draw.cpp` (lines 1811-1849)

**Key Logic:**

1. Find expected % from `::AzerothCore::expected_values.level_distribution`
2. Calculate deviation: `actual_percent - expected_percent`
3. If deviation > 3.0 → Green ▲
4. If deviation < -3.0 → Red ▼
5. Otherwise → No indicator

**Layout:**

- Bracket name: left-justified, 12 characters
- Arrow: 2 characters (arrow + space, or 2 spaces if none)
- Percentage: right-justified, 4 characters

## Benefits

✅ **At-a-glance monitoring** - Instantly see distribution health  
✅ **Non-intrusive** - Doesn't change text color, only adds small indicators  
✅ **Actionable** - Clear visual signal when adjustment needed  
✅ **Consistent colors** - All bracket names remain same grey color  
✅ **Informative** - Green/red provides intuitive direction feedback

## Build Status

✅ Successfully compiled (Dec 15, 2025)

- Build time: 1m 10s
- Binary: `/home/havoc/bottop/bin/bottop`

## Files Modified

- **`src/btop_draw.cpp`** (lines 1811-1849) - Added arrow indicator logic
