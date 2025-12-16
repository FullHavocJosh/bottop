# Comprehensive Ghosting Fix for Bottop UI

## Overview

Applied comprehensive ghosting prevention across all UI elements in bottop by clearing screen areas before redrawing dynamic content. This prevents text artifacts when content changes (e.g., server status changes, stats update, zone lists refresh).

## Areas Fixed

### 1. Performance Pane (Left Side)

#### Server URL (line 1619-1621)
```cpp
// Clear the line first to prevent ghosting
out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
```

#### Server Status (line 1625-1636)
```cpp
// Clear the entire line first to prevent ghosting
out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
out += Mv::to(cy++, perf_x + 2) + title + "Server Status: ";
if (data.status == ServerStatus::ONLINE) {
    out += Theme::c("proc_misc") + "ONLINE " + main_fg + "[" 
        + ::AzerothCore::format_uptime(data.stats.uptime_hours) + "]";
}
```

#### Ollama Stats Section (line 1650-1653)
```cpp
// Clear the Ollama section to prevent ghosting
for (int clear_line = 0; clear_line < 5; clear_line++) {
    out += Mv::to(ollama_cy + clear_line, ollama_x) + string(25, ' ');
}
```

#### WorldServer Performance Metrics (line 1672-1673)
```cpp
// Clear the line first to prevent ghosting
out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
```

#### Response Time Graph Area (line 1704-1707)
```cpp
// Clear the graph area to prevent ghosting
for (int clear_line = 0; clear_line < graph_height; clear_line++) {
    out += Mv::to(cy + clear_line, perf_x + 2) + string(perf_width - 4, ' ');
}
```

### 2. Distribution Pane (Right Side)

#### Entire Distribution Pane (line 1760-1763)
Clears factions, continents, and level brackets sections all at once:
```cpp
// Clear the entire distribution pane to prevent ghosting
for (int clear_line = 0; clear_line < dist_height - 2; clear_line++) {
    out += Mv::to(dist_y + 1 + clear_line, dist_x + 1) + string(dist_width - 2, ' ');
}
```

This single clear operation handles:
- **Factions**: Horde/Alliance distribution
- **Continents**: Eastern Kingdoms, Kalimdor, etc.
- **Levels**: All level brackets (1-10, 11-20, etc.)

### 3. Zones Pane (Bottom)

#### Entire Zones Pane (line 1876-1880)
```cpp
// Clear the entire zones pane to prevent ghosting
for (int clear_line = 0; clear_line < zones_height - 2; clear_line++) {
    out += Mv::to(zones_y + 1 + clear_line, x + 1) + string(width - 2, ' ');
}
cy = zones_y + 1;  // Reset cy after clearing
```

This handles:
- Zone list headers
- Zone data rows
- All zone statistics (bots, levels, alignment)
- Navigation help text

#### Bottom Navigation/Help Line (line 2107-2108)
```cpp
// Navigation help (clear bottom line first to prevent ghosting)
out += Mv::to(zones_y + zones_height - 1, x + 1) + string(width - 2, ' ');
```

This clears:
- Filter input box
- Navigation help text
- Timestamp display

## Technical Approach

### Strategy
1. **Clear-then-draw**: Always clear the area before writing new content
2. **Full area clearing**: Clear entire sections rather than individual lines when possible
3. **Coordinate-based**: Use `Mv::to()` with explicit coordinates for precise positioning
4. **Width-aware**: Use pane widths minus borders for clearing (e.g., `width - 2`)

### Benefits
- **No ghosting**: Old text is completely removed before new text is drawn
- **Clean transitions**: Smooth visual updates when content changes
- **Dynamic content safe**: Handles variable-length text properly
- **Performance**: Clearing is fast and doesn't impact frame rate

## Testing Scenarios

The ghosting fixes handle these common scenarios:

1. **Server status changes**: ONLINE → OFFLINE → RESTARTING
2. **Uptime updates**: Growing uptime values don't overlap
3. **Performance metrics**: UpdateTime values update cleanly
4. **Graph updates**: Historical data graphs refresh without artifacts
5. **Distribution changes**: Bot percentages update smoothly
6. **Zone list updates**: Adding/removing bots from zones
7. **Filter text**: Typing in zone filter doesn't ghost
8. **Navigation help**: Switching between filter/nav modes is clean

## Files Modified

- `src/btop_draw.cpp` - Added comprehensive clearing throughout draw function

## Build Results

```
Build complete in (01m:28s)
Binary: bin/bottop (1.9MiB)
```

## Before vs After

### Before
- Text would overlap when content changed
- Server status switching left artifacts
- Graph area showed ghost lines
- Distribution percentages would overlap
- Zone list had rendering artifacts

### After
- All content areas are cleared before redrawing
- Smooth, clean transitions between states
- No visual artifacts or text overlap
- Professional, polished appearance
