# Continent Distribution Color Diagnosis

## Problem

The Continent's % is **not being colored** based on its alignment with the expected bot distribution, while the Level brackets **are** being colored correctly.

## Root Cause

The continent distribution rendering code (lines 1837-1847 in `btop_draw.cpp`) does **not implement** the same color-based deviation logic that the level distribution code uses.

### Current Continent Code (Lines 1839-1846)

```cpp
for (const auto& continent : data.continents) {
    if (dist_cy >= dist_y + dist_height - 1) break;

    string percent_str = to_string((int)continent.percent) + "%";
    out += Mv::to(dist_cy, dist_x + 2) + Theme::c("inactive_fg") + ljust(continent.name, 15);
    out += main_fg + rjust(percent_str, 4);  // ← Always uses main_fg (no coloring)
    dist_cy++;
}
```

**Issue**: The continent name is always grey (`Theme::c("inactive_fg")`), and the percentage is always the default color (`main_fg`). There's **no logic** to check deviation from expected values.

### Level Bracket Code (Lines 1874-1913) - Works Correctly

```cpp
double percent = (it != data.levels.end()) ? it->percent : 0.0;

// Find expected percentage from server config for color indicator
double expected_percent = 0.0;
bool has_expected = false;
if (::AzerothCore::expected_values.loaded) {
    auto exp_it = std::find_if(::AzerothCore::expected_values.level_distribution.begin(),
                                ::AzerothCore::expected_values.level_distribution.end(),
                                [&bracket_name](const auto& lb) { return lb.range == bracket_name; });
    if (exp_it != ::AzerothCore::expected_values.level_distribution.end()) {
        expected_percent = exp_it->percent;
        has_expected = true;
    }
}

// 3-tier color system based on deviation from expected
// Green: ±0-1% (on target)
// Yellow: ±2-3% (warning)
// Red: >3% (critical)
string line_color = Theme::c("inactive_fg");  // Default grey
if (has_expected && expected_percent > 0) {
    double deviation = abs(percent - expected_percent);

    if (deviation <= 1.0) {
        // Green - on target (±1%)
        line_color = Theme::c("proc_misc");  // Bright green
    } else if (deviation <= 3.0) {
        // Yellow - warning (±2-3%)
        line_color = Theme::c("available_end");  // Bright yellow/orange
    } else {
        // Red - critical (>3%)
        line_color = Theme::c("used_end");  // Bright red
    }
}

string bracket_str = ljust(bracket_name, 15);
string percent_str = to_string((int)percent) + "%";

out += Mv::to(dist_cy, dist_x + 2) + line_color + bracket_str;
out += line_color + rjust(percent_str, 4);  // ← Uses calculated color
```

## Expected Values Are Loaded

The `ExpectedValues` structure (`btop_azerothcore.hpp:501-508`) contains:

```cpp
struct ExpectedValues {
    int bot_min = 0;
    int bot_max = 0;
    std::vector<LevelBracket> level_distribution;
    std::vector<Continent> continent_distribution;  // ← Expected values ARE available
    std::vector<BracketDefinition> bracket_definitions;
    bool loaded = false;
};
```

The `load_expected_values()` function (`btop_azerothcore.cpp:1879-1888`) correctly loads continent distribution:

```cpp
// Update continent distribution if found in config
if (!continent_percentages.empty()) {
    expected_values.continent_distribution.clear();
    for (const auto& pair : continent_percentages) {
        expected_values.continent_distribution.push_back({
            pair.first,  // name
            0,           // count (will be filled during collection)
            pair.second  // percent
        });
    }
    Logger::error("load_expected_values: Loaded " + to_string(continent_percentages.size()) + " continent percentages from config");
}
```

## Solution

The continent distribution rendering code needs to implement the **same 3-tier color deviation system** that level brackets use:

1. **Green** (±0-1%): On target
2. **Yellow** (±2-3%): Warning
3. **Red** (>3%): Critical deviation

### Required Changes to `btop_draw.cpp` (Lines 1839-1846)

Replace the current hardcoded coloring:

```cpp
string percent_str = to_string((int)continent.percent) + "%";
out += Mv::to(dist_cy, dist_x + 2) + Theme::c("inactive_fg") + ljust(continent.name, 15);
out += main_fg + rjust(percent_str, 4);
```

With deviation-based coloring:

```cpp
// Find expected percentage from server config for color indicator
double expected_percent = 0.0;
bool has_expected = false;
if (::AzerothCore::expected_values.loaded) {
    auto exp_it = std::find_if(::AzerothCore::expected_values.continent_distribution.begin(),
                                ::AzerothCore::expected_values.continent_distribution.end(),
                                [&continent](const auto& c) { return c.name == continent.name; });
    if (exp_it != ::AzerothCore::expected_values.continent_distribution.end()) {
        expected_percent = exp_it->percent;
        has_expected = true;
    }
}

// 3-tier color system based on deviation from expected
string line_color = Theme::c("inactive_fg");  // Default grey
if (has_expected && expected_percent > 0) {
    double deviation = abs(continent.percent - expected_percent);

    if (deviation <= 1.0) {
        line_color = Theme::c("proc_misc");  // Green - on target
    } else if (deviation <= 3.0) {
        line_color = Theme::c("available_end");  // Yellow - warning
    } else {
        line_color = Theme::c("used_end");  // Red - critical
    }
}

string percent_str = to_string((int)continent.percent) + "%";
out += Mv::to(dist_cy, dist_x + 2) + line_color + ljust(continent.name, 15);
out += line_color + rjust(percent_str, 4);
```

## Files to Modify

- **`src/btop_draw.cpp`** (lines 1839-1846) - Add deviation-based coloring logic

## Summary

The continent distribution is **missing** the deviation-based coloring logic that the level brackets have. The expected values are loaded correctly from the config, but the drawing code doesn't use them. Adding the same 3-tier color system (green/yellow/red based on deviation) will fix the issue.
