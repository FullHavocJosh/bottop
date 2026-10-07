# Color Visibility Fix - Level Distribution

## Problem

The 3-tier color system for level distribution brackets was not visible because the theme colors being used were not appropriate for text display:

- `available` returned a gradient start color (dark brown #4e3f0e instead of green)
- `graph_text` was dark grey (#60 instead of yellow)
- `title` was bright white (#ee instead of red)
- `hi_fg` was dull red (#b54040 instead of bright)

## Solution

Changed to use theme colors that are designed for bright, visible text:

### New Color Mapping

```cpp
// Green (±0-3% deviation): proc_misc
line_color = Theme::c("proc_misc");  // Bright green #0de756

// Yellow (±3-6% deviation): available_end
line_color = Theme::c("available_end");  // Bright yellow/orange #ffb814

// Red (±6-9% deviation): used_end
line_color = Theme::c("used_end");  // Bright red #ff4769

// White (>9% deviation): title
line_color = Theme::c("title");  // Bright white #ee
```

## Files Modified

- `src/btop_draw.cpp` (lines 1837, 1840, 1843, 1846)

## Color Examples (Default Theme)

- **Green**: #0de756 (bright lime green - used for "on target")
- **Yellow**: #ffb814 (bright orange/yellow - used for "warning")
- **Red**: #ff4769 (bright coral red - used for "critical")
- **White**: #ee (bright white - used for "severe")
- **Grey**: #40 (dark grey - used for no expected values)

## Testing

Run `./bin/bottop` and observe the Level Distribution section:

- Brackets within expected ranges should show bright green
- Brackets with moderate deviation should show bright yellow/orange
- Brackets with high deviation should show bright red
- Brackets with extreme deviation should show bright white

## Color Scheme Logic

The color indicates how far the actual bot distribution deviates from the expected distribution configured on the server:

| Deviation | Color      | Meaning     | Theme Key       |
| --------- | ---------- | ----------- | --------------- |
| 0-3%      | **Green**  | On target ✓ | `proc_misc`     |
| 3-6%      | **Yellow** | Warning ⚠   | `available_end` |
| 6-9%      | **Red**    | Critical ✗  | `used_end`      |
| >9%       | **White**  | Severe!     | `title`         |

## Build Command

```bash
make -j$(nproc) STATIC= GPU_SUPPORT=false RSMI_STATIC= ADDFLAGS="-DAZEROTHCORE_SUPPORT"
```

Build time: ~1 minute on 12-core system

## Previous Implementation

The previous implementation used:

- `available` (gradient start, dark brown)
- `graph_text` (dark grey)
- `title` (for red, but was white)
- `hi_fg` (dull red for bright)

These were not suitable for text coloring and resulted in poor visibility.
