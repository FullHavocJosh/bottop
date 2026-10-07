# Stock btop Boxes Disabled - Implementation Summary

## Changes Made

### 1. Hard-Coded All Stock Boxes to false (btop_draw.cpp:2096-2122)

**Before:**

```cpp
Cpu::shown = boxes.contains("cpu");
Mem::shown = boxes.contains("mem");
Net::shown = boxes.contains("net");
Proc::shown = boxes.contains("proc");
Gpu::shown = Gpu::shown_panels.size();
```

**After:**

```cpp
// Disable all stock btop boxes - bottop only uses AzerothCore box
Cpu::shown = false;  // Disabled: boxes.contains("cpu");
Mem::shown = false;  // Disabled: boxes.contains("mem");
Net::shown = false;  // Disabled: boxes.contains("net");
Proc::shown = false; // Disabled: boxes.contains("proc");
Gpu::shown = 0;      // Disabled: Gpu::shown_panels.size();
```

**Result:**

- Even if the config file specifies other boxes, they will NOT be shown
- Only the AzerothCore box can be displayed
- GPU initialization code also commented out to prevent any GPU processing

### 2. Updated Config File

**File:** `~/.config/bottop/bottop.conf`

**Content:**

```
shown_boxes = "azerothcore"  #* NOTE: cpu, mem, net, proc, and gpu boxes are disabled in bottop code
```

This serves as documentation that other boxes are intentionally disabled.

## What This Prevents

### Before (Issues)

1. ❌ Config file could enable CPU box: `shown_boxes = "azerothcore cpu"`
2. ❌ User could toggle boxes with number keys (1-9)
3. ❌ Menu options could enable other boxes
4. ❌ Stock btop boxes would render behind/over AzerothCore box
5. ❌ Keybinds from other boxes would interfere (like 'z' in Net box)

### After (Fixed)

1. ✅ **CPU box always disabled** - `Cpu::shown = false`
2. ✅ **Memory box always disabled** - `Mem::shown = false`
3. ✅ **Network box always disabled** - `Net::shown = false`
4. ✅ **Process box always disabled** - `Proc::shown = false`
5. ✅ **GPU boxes always disabled** - `Gpu::shown = 0`
6. ✅ **AzerothCore box only** - Clean display, no bleeding through
7. ✅ **No keybind conflicts** - Only q, esc, z active

## Code Locations

### Draw Functions Affected

Since `shown` flags are false, these draw functions will NOT execute:

- `Cpu::draw()` - Skipped
- `Mem::draw()` - Skipped
- `Net::draw()` - Skipped
- `Proc::draw()` - Skipped
- `Gpu::draw()` - Skipped

Only `AzerothCore::draw()` will execute.

### Input Handlers Affected

Combined with the `return;` at line 337 of `btop_input.cpp`:

- CPU input handlers - Unreachable
- Memory input handlers - Unreachable
- Network input handlers - Unreachable (including 'z' key for reset)
- Process input handlers - Unreachable
- GPU input handlers - Unreachable

## Build Status

✅ **Build successful** - No errors, no warnings

## Testing Verification

### 1. Start bottop

```bash
cd /home/havoc/bottop
./build/bottop
```

### 2. Expected Display

**ONLY see:**

- AzerothCore Performance Graph (top)
- Zone List (bottom)

**SHOULD NOT see:**

- CPU usage graphs
- Memory bars
- Network graphs
- Process list
- GPU information
- Any background elements

### 3. Test Keybinds

- `z` - Should activate zone navigation (NOT turn elements red)
- `q` - Should quit
- `Esc` - Should open menu
- `1-9` - Should do nothing (box toggles disabled)
- `p/P` - Should do nothing (presets disabled)
- `Arrow keys` - Should navigate zones (not change sorting/selection in other boxes)

### 4. Test Menu

Even if you try to enable other boxes through the menu options, they will remain disabled because the `shown` flags are hard-coded to false.

## Comparison: btop vs bottop

| Feature                | btop   | bottop          |
| ---------------------- | ------ | --------------- |
| CPU monitoring         | ✅ Yes | ❌ Disabled     |
| Memory monitoring      | ✅ Yes | ❌ Disabled     |
| Network monitoring     | ✅ Yes | ❌ Disabled     |
| Process list           | ✅ Yes | ❌ Disabled     |
| GPU monitoring         | ✅ Yes | ❌ Disabled     |
| AzerothCore monitoring | ❌ No  | ✅ Only feature |
| Active keybinds        | Many   | 3 (q, esc, z)   |
| Zone navigation        | ❌ No  | ✅ Yes          |
| Bot tracking           | ❌ No  | ✅ Yes          |
| Level distribution     | ❌ No  | ✅ Yes          |

## Files Modified

1. **src/btop_draw.cpp** (lines 2096-2122)
    - Hard-coded all stock box `shown` flags to false
    - Commented out GPU initialization code

2. **~/.config/bottop/bottop.conf**
    - Set `shown_boxes = "azerothcore"`
    - Added comment documenting disabled boxes

## Rollback Instructions (if needed)

To re-enable stock btop functionality:

1. **Restore draw.cpp:**

```cpp
Cpu::shown = boxes.contains("cpu");
Mem::shown = boxes.contains("mem");
Net::shown = boxes.contains("net");
Proc::shown = boxes.contains("proc");
```

2. **Remove return in input.cpp:**
   Comment out line 337: `// return;`

3. **Update config:**

```
shown_boxes = "cpu mem net proc"
```

4. **Rebuild:**

```bash
cd /home/havoc/bottop
cmake --build build
```

## Notes

- This is a permanent code-level change, not just a config change
- Config file setting is now essentially documentation only
- The disabled boxes consume no CPU/memory since their draw functions never execute
- Input handlers for disabled boxes are also unreachable, saving processing time
- bottop is now truly minimal: AzerothCore monitoring only

## Success Criteria

✅ Only AzerothCore box visible
✅ No CPU/Mem/Net/Proc/GPU boxes render
✅ No background elements bleeding through  
✅ 'z' key activates zone navigation cleanly
✅ No red elements or interference from other boxes
✅ Clean, focused interface for AzerothCore bot monitoring
