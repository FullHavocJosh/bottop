# Keybind Fix Summary

## Issues Found & Fixed

### Issue 1: Config File Had CPU Box Enabled

**Problem**: User's config file at `~/.config/bottop/bottop.conf` had:

```
shown_boxes = "azerothcore cpu"
```

This was causing the CPU box to render behind the AzerothCore pane.

**Fix**: Updated config file to:

```
shown_boxes = "azerothcore"
```

**Location**: `/home/havoc/.config/bottop/bottop.conf`

### Issue 2: 'z' Key Turning Elements Red

This was likely the CPU box responding to the 'z' key (which is used in Net box for resetting stats), before our input handler could intercept it.

With the CPU box disabled, this should no longer happen since our input handler now returns early at line 337, preventing any other box handlers from executing.

## Code Changes Summary

### 1. Zone Header - Shows Navigation Hint (btop_draw.cpp:2623-2626)

```cpp
string zone_hint = zone_selection_active ? "" : Theme::c("graph_text") + " [press z to navigate zones]";
out += Mv::to(cy, x + 2) + title + "Zone" + zone_hint + ...
```

### 2. Removed Escape from Zone Exit (btop_input.cpp:290)

```cpp
if (key == "left" or (vim_keys and key == "h")) {  // Removed "escape" from here
```

### 3. Updated Help Text (btop_draw.cpp:2707)

```cpp
+ "↑↓:Navigate  Enter/→:Expand  ←:Exit  Space:Toggle"  // Changed "Esc/←:Exit" to "←:Exit"
```

### 4. Disabled All Stock btop Input Handlers (btop_input.cpp:336-337)

```cpp
#endif

	// Disable all other stock btop input handlers - bottop only uses q, esc, and z
	return;

	//? Input actions for proc box
	if (Proc::shown) {
		// ... all this code is now unreachable
```

### 5. Fixed Z Key Indentation (btop_input.cpp:328-333)

Fixed indentation so the zone activation code is properly inside the else-if block.

## Active Keybinds

### Global Keys

- `q` - Quit bottop
- `Esc` / `m` - Open main menu

### Zone Navigation Keys (when zones visible)

- `z` - Enter zone navigation mode

### In Zone Navigation Mode

- `↑` / `k` - Move selection up
- `↓` / `j` - Move selection down
- `Enter` / `→` / `l` / `Space` - Toggle zone expansion (show/hide level details)
- `←` / `h` - Exit zone navigation mode

## Testing Instructions

### 1. Start bottop

```bash
cd /home/havoc/bottop
./build/bottop
```

### 2. Verify Only AzerothCore Box Shows

- You should see ONLY the AzerothCore performance and zones panes
- NO CPU, Memory, Network, or Process boxes
- No background elements bleeding through

### 3. Test Zone Navigation

1. Press `z` - First zone should highlight (NOT turn red)
2. Press `↓` - Selection moves down
3. Press `↑` - Selection moves up
4. Press `Enter` - Zone expands, showing level distribution
5. Press `Enter` again - Zone collapses
6. Press `←` - Exit navigation mode (selection highlighting disappears)

### 4. Test Menu

1. Press `Esc` - Main menu should open
2. Use menu navigation
3. Exit menu

### 5. Test Quit

1. Press `q` - bottop should exit cleanly

## Expected Behavior

### Before Pressing 'z'

```
Zone [press z to navigate zones]    Bots    Min Lvl    Max Lvl    Align
─────────────────────────────────────────────────────────────────────────
● ▶ Orgrimmar                        150      1          80        89%
● ▶ Stormwind City                   120      1          80        78%
```

### After Pressing 'z'

```
Zone                                 Bots    Min Lvl    Max Lvl    Align
─────────────────────────────────────────────────────────────────────────
● ▶ Orgrimmar                        150      1          80        89%  ← HIGHLIGHTED
● ▶ Stormwind City                   120      1          80        78%

↑↓:Navigate  Enter/→:Expand  ←:Exit  Space:Toggle
```

### After Pressing Enter (Zone Expanded)

```
Zone                                 Bots    Min Lvl    Max Lvl    Align
─────────────────────────────────────────────────────────────────────────
● ▼ Orgrimmar                        150      1          80        89%  ← HIGHLIGHTED
    Level 1-10                        35                            23%
    Level 11-20                       50                            33%
    Level 71-80                       65                            43%
● ▶ Stormwind City                   120      1          80        78%

↑↓:Navigate  Enter/→:Expand  ←:Exit  Space:Toggle
```

## Files Modified

1. `src/btop_draw.cpp` - Header text, help text
2. `src/btop_input.cpp` - Keybind handling, disabled unused handlers
3. `~/.config/bottop/bottop.conf` - Removed "cpu" from shown_boxes

## Build Status

✅ Build successful - No errors, no warnings

## If Issues Persist

### CPU Box Still Showing Through

Check the config file:

```bash
cat ~/.config/bottop/bottop.conf | grep shown_boxes
```

Should output: `shown_boxes = "azerothcore"`

If not, manually edit:

```bash
nano ~/.config/bottop/bottop.conf
```

Change to: `shown_boxes = "azerothcore"`

### 'z' Key Still Turns Things Red

1. Check if Cpu::shown or Net::shown is true
2. Verify the `return;` at line 337 of btop_input.cpp is executing
3. Add debug output before the return to verify execution path

### Zone Navigation Not Working

1. Verify `Draw::AzerothCore::shown` is true
2. Check database connection is working (zones should be visible)
3. Add debug output in the 'z' key handler (line 328-333)
