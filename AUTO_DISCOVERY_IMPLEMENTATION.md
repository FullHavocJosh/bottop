# Auto-Discovery Implementation

**Date**: Dec 16, 2024 18:52 EST  
**Status**: ✅ Complete - Built Successfully

## What Was Implemented

Added automatic discovery of AzerothCore container names and config file paths, eliminating the need for manual configuration in `~/.config/bottop/bottop.conf`.

## Changes Made

**File**: `src/btop_azerothcore.cpp` (lines 1703-1797)

### 1. Updated Early Return Logic (line 1703-1710)

**Before**:

```cpp
if (!ssh_client || !ssh_client->is_connected() || config.config_path.empty()) {
    // Return if SSH not available OR config_path empty
    return;
}
```

**After**:

```cpp
// Check if SSH is available at all
if (!ssh_client || !ssh_client->is_connected()) {
    // Only return if SSH not available - allow empty config_path for auto-discovery
    return;
}
```

**Impact**: Removed `config.config_path.empty()` check to allow auto-discovery when config is not manually set.

### 2. Container Auto-Discovery (lines 1712-1755)

Tries multiple patterns to find the worldserver container:

```cpp
std::vector<std::string> container_patterns = {
    "docker ps --filter 'name=ac-worldserver' --format '{{.Names}}' | head -1",
    "docker ps --filter 'name=worldserver' --format '{{.Names}}' | grep -i ac | head -1",
    "docker ps --format '{{.Names}}' | grep -i worldserver | head -1",
    "docker ps --format '{{.Names}}' | grep 'ac-' | head -1"
};
```

**Logic**:

1. If `config.container` is empty, attempt auto-discovery
2. Try each pattern in order via SSH
3. Trim whitespace/newlines from result
4. Use first non-empty, non-error result
5. If all patterns fail, return (use defaults)
6. If `config.container` already set, skip auto-discovery

**Logging**:

- "Container not configured, attempting auto-discovery..."
- "Trying pattern: [command]"
- "Auto-discovered container: [name]" OR "Failed to auto-discover container"
- "Using manually configured container: [name]" (if pre-configured)

### 3. Config Path Auto-Discovery (lines 1757-1797)

Tries common config file locations in order:

```cpp
std::vector<std::string> config_paths = {
    "/azerothcore/env/dist/etc/worldserver.conf",
    "/etc/worldserver.conf",
    "/opt/azerothcore/etc/worldserver.conf",
    "/azerothcore/etc/worldserver.conf"
};
```

**Logic**:

1. If `config.config_path` is empty, attempt auto-discovery
2. For each path, test if file exists: `docker exec <container> test -f <path> && echo found`
3. Use first path that returns "found"
4. If all paths fail, return (use defaults)
5. If `config.config_path` already set, skip auto-discovery

**Logging**:

- "Config path not configured, attempting auto-discovery..."
- "Testing path: [path]"
- "Auto-discovered config path: [path]" OR "Failed to auto-discover config path"
- "Using manually configured config path: [path]" (if pre-configured)

## How It Works

### Execution Flow

```
1. load_expected_values() called
   ↓
2. Set default brackets/continents
   ↓
3. Check SSH connection
   - If no SSH: return (use defaults)
   ↓
4. Container auto-discovery (if config.container empty)
   - Try pattern 1: filter 'name=ac-worldserver'
   - Try pattern 2: filter 'name=worldserver' + grep ac
   - Try pattern 3: grep worldserver
   - Try pattern 4: grep 'ac-'
   - If found: set config.container
   - If not found: return (use defaults)
   ↓
5. Config path auto-discovery (if config.config_path empty)
   - Try: /azerothcore/env/dist/etc/worldserver.conf
   - Try: /etc/worldserver.conf
   - Try: /opt/azerothcore/etc/worldserver.conf
   - Try: /azerothcore/etc/worldserver.conf
   - If found: set config.config_path
   - If not found: return (use defaults)
   ↓
6. Proceed with config file parsing (existing code)
```

### Manual vs Auto Configuration

**Manual Config** (still supported):

```ini
# ~/.config/bottop/bottop.conf
azerothcore_container = "my-custom-container"
azerothcore_config_path = "/custom/path/worldserver.conf"
```

- Auto-discovery skipped if these are set
- Logs: "Using manually configured container/config_path"

**Auto Config** (new):

```ini
# ~/.config/bottop/bottop.conf
# Leave empty or omit:
azerothcore_container = ""
azerothcore_config_path = ""
```

- Auto-discovery runs on every `load_expected_values()` call
- Logs: "Auto-discovered container/config_path"

## Error Handling

### Defensive Programming

- Each pattern/path wrapped in try-catch
- Failures logged but don't crash - continue to next option
- Final fallback: use default brackets/continents (grey display)

### Failure Scenarios

**Container discovery fails**:

```
Log: "Failed to auto-discover container, using defaults"
Result: Level brackets show grey percentages (defaults)
```

**Config path discovery fails**:

```
Log: "Failed to auto-discover config path, using defaults"
Result: Level brackets show grey percentages (defaults)
```

**SSH connection fails**:

```
Log: "No SSH connection available, using defaults"
Result: Level brackets show grey percentages (defaults)
```

## Testing Instructions

### Test 1: Auto-Discovery (Empty Config)

```bash
# Clear manual config
grep -v "azerothcore_container\|azerothcore_config_path" ~/.config/bottop/bottop.conf > /tmp/conf.tmp
mv /tmp/conf.tmp ~/.config/bottop/bottop.conf

# Run bottop
/Users/havoc/bottop/bin/bottop

# Check logs
tail -f /tmp/bottop_expected_values_debug.log
grep "Auto-discovered" /tmp/bottop_expected_values_debug.log

# Expected logs:
# - "Container not configured, attempting auto-discovery..."
# - "Auto-discovered container: testing-ac-worldserver"
# - "Auto-discovered config path: /azerothcore/env/dist/etc/worldserver.conf"
```

**Visual indicators of success**:

- Level brackets show colored percentages (green/yellow/red)
- Continents show colored percentages
- Grey percentages = auto-discovery failed

### Test 2: Manual Config (Should Still Work)

```bash
# Set manual config
echo 'azerothcore_container = "my-container"' >> ~/.config/bottop/bottop.conf
echo 'azerothcore_config_path = "/custom/worldserver.conf"' >> ~/.config/bottop/bottop.conf

# Run bottop
/Users/havoc/bottop/bin/bottop

# Check logs
grep "Using manually configured" /tmp/bottop_expected_values_debug.log

# Expected logs:
# - "Using manually configured container: my-container"
# - "Using manually configured config path: /custom/worldserver.conf"
```

### Test 3: Check Current Status

```bash
# View current config
cat ~/.config/bottop/bottop.conf | grep azerothcore

# Run bottop and watch logs in real-time
tail -f /tmp/bottop_expected_values_debug.log &
/Users/havoc/bottop/bin/bottop
```

## Expected Behavior for User's Setup

**User's server**: `root@testing-azerothcore.rollet.family`  
**Expected container**: `testing-ac-worldserver` or similar  
**Expected config path**: `/azerothcore/env/dist/etc/worldserver.conf`

### What Will Happen

1. SSH connects to remote server (already working)
2. Container auto-discovery:
    - Tries: `docker ps --filter 'name=ac-worldserver'`
    - Finds: `testing-ac-worldserver`
    - Sets: `config.container = "testing-ac-worldserver"`
3. Config path auto-discovery:
    - Tries: `docker exec testing-ac-worldserver test -f /azerothcore/env/dist/etc/worldserver.conf`
    - Returns: "found"
    - Sets: `config.config_path = "/azerothcore/env/dist/etc/worldserver.conf"`
4. Loads worldserver.conf (existing code)
5. Loads bracket config from `/azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf`
6. Parses expected values for level brackets, continents
7. Display shows colored percentages (green/yellow/red)

## Build Info

**Build Command**: `cd /Users/havoc/bottop && make -j8`  
**Build Time**: ~5 seconds  
**Binary**: `/Users/havoc/bottop/bin/bottop` (2.7 MiB)  
**Status**: ✅ Compiles successfully  
**Warnings**: 1 unused lambda capture warning (pre-existing, line 1993)

## Files Modified

- `src/btop_azerothcore.cpp` (lines 1703-1797)

## Files Not Modified

- No changes to config file format
- No changes to data structures
- No changes to SSH connection logic
- No changes to parsing logic

## Backward Compatibility

✅ **Fully backward compatible**:

- Manual config still works if set
- Existing installations unaffected
- Falls back to defaults gracefully
- No breaking changes to APIs or data structures

## Next Steps

1. **User testing on actual server**
2. **Monitor logs**: `/tmp/bottop_expected_values_debug.log`
3. **Verify colored displays**: Level brackets and continents should show colors, not grey
4. **Optional**: Add more container patterns if needed based on user's actual container name

## Success Criteria

- [x] Compiles without errors
- [x] Container auto-discovery implemented
- [x] Config path auto-discovery implemented
- [x] Comprehensive logging added
- [x] Error handling implemented
- [x] Manual config still supported
- [x] Graceful fallback to defaults
- [ ] User testing on remote server (pending)
- [ ] Verify colored displays in production (pending)
