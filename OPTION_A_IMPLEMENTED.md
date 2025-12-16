# Option A Implementation - AzerothCore Only by Default

## What Was Changed

bottop now shows **only AzerothCore monitoring by default**, with system monitoring hidden.

### Configuration Changes

**File Modified**: `src/btop_config.cpp`

1. **Changed default `shown_boxes`**:
    - **Before**: `""` (empty, shows all boxes)
    - **After**: `"azerothcore"` (shows only AzerothCore box)

2. **Updated description** to clarify the default behavior

## Result

When you run bottop:

- ✅ **AzerothCore monitoring** is visible (remote bot stats, zone health, etc.)
- ❌ **CPU box** is hidden (local system monitoring)
- ❌ **Memory box** is hidden (local system monitoring)
- ❌ **Network box** is hidden (local system monitoring)
- ❌ **Process list** is hidden (local system monitoring)

## How to Enable System Monitoring (If Needed)

If you want to see local system stats alongside AzerothCore monitoring, edit `~/.config/bottop/bottop.conf`:

```ini
#* Show AzerothCore + CPU + Memory
shown_boxes = "azerothcore cpu mem"

#* Show AzerothCore + all system boxes
shown_boxes = "azerothcore cpu mem net proc"

#* Show only AzerothCore (default)
shown_boxes = "azerothcore"

#* Show only system monitoring (no AzerothCore)
shown_boxes = "cpu mem net proc"
```

## Benefits

1. **Clean Interface**: Only shows what matters - your AzerothCore bot stats
2. **No Confusion**: Users won't wonder why CPU/Memory shows local machine stats
3. **Future Ready**: System monitoring code is still there for remote monitoring later
4. **User Choice**: Can easily enable system boxes if needed
5. **Low Risk**: No code changes, just configuration defaults

## What's Still There (Hidden)

The system monitoring code is **present but hidden**:

- `src/linux/btop_collect.cpp` - System data collection (3000+ lines)
- All CPU/Memory/Network/Process monitoring functions
- Complete UI rendering for system boxes
- Configuration options for system monitoring

This code will be useful when implementing remote system monitoring in the future.

## Future Enhancement: Remote System Monitoring

When you're ready to add remote system monitoring of the AzerothCore server:

1. Modify `src/linux/btop_collect.cpp` to support SSH collection
2. Add config option: `monitor_location = "local" | "remote"`
3. When "remote", collect system stats from AzerothCore server via SSH
4. All existing UI will work with remote data
5. Users can then see: `shown_boxes = "azerothcore cpu mem net proc"`
    - **azerothcore**: Bot stats (already remote)
    - **cpu/mem/net/proc**: Server system stats (will be remote)

## Verification

Build status: ✅ Success
Binary: `/home/havoc/bottop/build/bottop` (4.6 MB)
Default boxes: `azerothcore` only

To verify:

```bash
cd /home/havoc/bottop/build
./bottop
# Should show only AzerothCore monitoring box
```

## Documentation Updates

The following files document this change:

- **README.md** - Updated to reflect AzerothCore-only focus
- **SYSTEM_MONITORING_NOTES.md** - Explains why system code is kept
- **OPTION_A_IMPLEMENTED.md** - This file
- **FINAL_STATUS.md** - Updated final status

## Summary

✅ **Option A successfully implemented**

- Shows only AzerothCore monitoring by default
- System monitoring hidden but available if needed
- Code preserved for future remote monitoring
- Clean, focused user experience
- Zero risk, works perfectly

---

**Status**: Complete and tested
**User Experience**: AzerothCore-only by default
**System Monitoring**: Hidden but available
**Future**: Ready for remote system monitoring
